// A small tool that connects to keyboard via libusb and listens for
// its activity.
// Details are designed for a particular vendor & product,
// for Q&D implementation.
// It releases all and stops after 5 seconds of inactivity.
// Tested under Ubuntu 14.04.

// (C) 2016 Valentin Nechayev <netch80@github.com>
// License: public domain

#include <libusb-1.0/libusb.h>
//#include <usb.h>
#include <time.h>
#include <stdio.h>
#include <err.h>

struct UsbContext {
  libusb_context *ctx = nullptr;
  ~UsbContext() {
    if (ctx) {
      libusb_exit(ctx);
      ctx = nullptr;
    }
  }
  void init() {
    int r = libusb_init(&ctx);
    if (r != 0) {
      errx(1, "libusb_init() -> %d", r);
    }
  }
};

struct UsbDeviceList {
  libusb_device** dlist = nullptr;
  ssize_t len = 0;
  ~UsbDeviceList() {
    if (dlist) {
      libusb_free_device_list(dlist, 1);
      dlist = nullptr;
      len = 0;
    }
  }
  void fetch(UsbContext* uctx) {
    len = libusb_get_device_list(uctx->ctx, &dlist);
  }
};

struct UsbDeviceHandle {
  libusb_device_handle *handle = nullptr;
  ~UsbDeviceHandle() {
    if (handle != nullptr) {
      libusb_close(handle);
      handle = nullptr;
    }
  }
  int open(libusb_device* d) {
    return libusb_open(d, &handle);
  }
};

struct UsbClaimInterface {
  libusb_device_handle *handle = nullptr;
  int interface;
  ~UsbClaimInterface() {
    if (handle) {
      libusb_release_interface(handle, interface);
      handle = nullptr;
    }
  }
  int claim(libusb_device_handle *newh, int newinterface) {
    int r;
    r = libusb_claim_interface(newh, newinterface);
    if (!r) {
      handle = newh;
      interface = newinterface;
    }
    return r;
  }
};

struct UsbDetachDriver {
  libusb_device_handle* handle = nullptr;
  int interface;
  ~UsbDetachDriver() {
    if (handle) {
      libusb_attach_kernel_driver(handle, interface);
      handle = nullptr;
    }
  }
  int detach(libusb_device_handle* newh, int newinterface) {
    int r;
    r = libusb_detach_kernel_driver(newh, newinterface);
    if (!r) {
      handle = newh;
      interface = newinterface;
    }
    return r;
  }
};

//--------------------------------------------------------------

double etime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1.0e-6;
}

static void
process_device(libusb_device *d)
{
  int r, cfg;
  UsbDeviceHandle uh;
  struct libusb_device_descriptor dd;
  struct libusb_config_descriptor *cdp;

  r = uh.open(d);
  if (r != 0) {
    printf("  libusb_open() -> %d\n", r);
    return;
  }
  r = libusb_get_device_descriptor(d, &dd);
  if (r == 0) {
    if (dd.bDeviceClass != 0 && dd.bDeviceClass != 3) {
      return;
    }
    if (dd.idVendor != 0x0566 || dd.idProduct != 0x3002) {
      return;
    }
    printf("  device descriptor:\n");
    printf("    length=%u type=%u\n", dd.bLength, dd.bDescriptorType);
    printf("    hwtype=%04X:%04X class=%02X subclass=%02X\n",
        dd.idVendor, dd.idProduct, dd.bDeviceClass, dd.bDeviceSubClass);
  }
  else {
    printf("  libusb_get_device_descriptor() -> %d\n", r);
    return;
  }
  r = libusb_get_configuration(uh.handle, &cfg);
  if (r == 0)
    printf("  active configuration: %d\n", cfg);
  else
    printf("  libusb_get_configuration() -> %d\n", r);
  r = libusb_get_active_config_descriptor(d, &cdp);
  if (r == 0) {
    printf("  active config descriptor:\n"
           "    bNumInterfaces=%u bConfigurationValue=%u\n"
           "    iConfiguration=%u\n"
           "    bmAttributes=0x%02X MaxPower=%u\n",
           cdp->bNumInterfaces, cdp->bConfigurationValue,
           cdp->iConfiguration,
           cdp->bmAttributes, cdp->MaxPower
      );
    libusb_free_config_descriptor(cdp);
  }
  else {
    printf("  libusb_get_config_descriptor(%d) -> %d\n", cfg, r);
  }
  if (libusb_kernel_driver_active(uh.handle, 0)) {
    printf("  kernel driver is active\n");
    //-return;
  }
  UsbDetachDriver udd;
  r = udd.detach(uh.handle, 0);
  if (r != 0) {
    printf("  libusb_detach_kernel_driver() -> [%d]%s\n", r, libusb_error_name(r));
    return;
  }
  printf("  detach succeeded\n");
  UsbClaimInterface uclaim;
  r = uclaim.claim(uh.handle, 0);
  if (r != 0) {
    printf("  libusb_claim_interface() -> [%d]%s\n", r, libusb_error_name(r));
    return;
  }
  printf("  claim succeeded\n");
  unsigned char buf[8];
  double expiry = etime() + 5;
  for(;;) {
    double now = etime();
    int transferred;
    if (now >= expiry) { break; }
    unsigned timo = (unsigned) 1000 * (expiry - now);
    transferred = 0;
    r = libusb_interrupt_transfer(uh.handle, 0x81, buf, 8, &transferred, timo);
    if (transferred > 0) {
      printf("  Got data:");
      for (int u = 0; u < transferred; ++u) {
        printf(" %02X", 255 & buf[u]);
      }
      putchar('\n');
      expiry = etime() + 5;
    }
    if (r != 0) {
      if (r != LIBUSB_ERROR_TIMEOUT) {
        printf("  libusb_interrupt_transfer -> [%d]%s\n", r, libusb_error_name(r));
      }
      break;
    }
  } // reading cycle
}

int
main()
{
  int i;

  UsbContext uctx;
  uctx.init();
  libusb_set_debug(uctx.ctx, 3);
  UsbDeviceList udlist;
  udlist.fetch(&uctx);
  printf("Found %zd devices\n", udlist.len);
  for (i = 0; i < udlist.len; ++i) {
    printf("List item %d:\n", i);
    process_device(udlist.dlist[i]);
  }

  return 0;
}

// vim:ts=2:sts=2:sw=2:et:si:
