#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <err.h>
#include <sys/soundcard.h>
#include "defs.h"

int target;

const char *snd_dev_name = "/dev/dsp"; // default is /dev/dsp
int snd_dev = -1; // device descriptor

void
audio_open(void)
{
  if (ofilepath) {
    target = TARGET_FILE;
    ofile = fopen(ofilepath, "w");
    if (!ofile)
      err(1, "fopen(ofile)");
    snd_dev = fileno(ofile); // for write()
  }
  else {
    target = TARGET_DEVICE;
    snd_dev = open(snd_dev_name, O_WRONLY, 0);
    if (snd_dev == -1)
      err(1, "open(audiodevice)");
  }
}

// Speed probe. Format and channels shall be already configured.
// Returned value: 0 - failed, 1 - OK exactly, 2 - OK approximately
int
setSpeed(int speed, int *rcp, int *newspeedp)
{
  int val = speed;
  double d;
  if (ioctl(snd_dev, SNDCTL_DSP_SPEED, &val) < 0) {
    if (rcp != NULL)
      *rcp = errno;
    return 0;
  }
  if (val == 0) {
    if (rcp != NULL)
      *rcp = EIO;
    return 0;
  }
  if (newspeedp != NULL)
    *newspeedp = val;
  if (val == speed)
    return 1;
  d = (speed*1.0)/val;
  if (d >= 0.95 && d <= 1.05)
    return 2;
  if (rcp != NULL)
    *rcp = ENOENT;
  return 0;
}

int
cmdProbe(void)
{
  int rc, mask, val;
  struct snd_size sndsize;

  if (target == TARGET_FILE) {
    printf("For file, there is no hardware capabilities.\n");
    printf("File is written with: 48kHz/16bit/mono.\n");
    return 0;
  }

  printf("Checking hardware capabilities\n");
  if (ioctl(snd_dev, SNDCTL_DSP_GETFMTS, &mask) == 0) {
    int cnt = 0;
    printf("Supported audio formats:");
#ifdef AFMT_U8
    if (mask & AFMT_U8) {
      printf(" U8");
      ++cnt;
    }
#endif
#ifdef AFMT_S8
    if (mask & AFMT_S8) {
      printf(" S8");
      ++cnt;
    }
#endif
#ifdef AFMT_S16_LE
    if (mask & AFMT_S16_LE) {
      printf(" S16_LE");
      ++cnt;
    }
#endif
#ifdef AFMT_S16_BE
    if (mask & AFMT_S16_BE) {
      printf(" S16_BE");
      ++cnt;
    }
#endif
    printf(" (%d total of interested)\n", cnt);
  }
  else {
    warn("ioctl(SNDCTL_DSP_GETFMTS)");
  }
  printf("Selecting S16_NE and mono\n");
  // Required order is: format, channels, speed
  val = AFMT_S16_NE;
  if (ioctl(snd_dev, SNDCTL_DSP_SETFMT, &val) < 0)
    err(1, "ioctl(,SNDCTL_DSP_SETFMT, AFMT_S16_NE)");
  if (val != AFMT_S16_NE)
    errx(1, "fatal: device does not support AFMT_S16_NE");
  val = 1;
  if (ioctl(snd_dev, SNDCTL_DSP_CHANNELS, &val) < 0)
    err(1, "ioctl(,SNDCTL_DSP_CHANNELS, 1");
  if (val != 1)
    errx(1, "fatal: device does not support mono");
  // Probe speed
  val = 0;
  if (ioctl(snd_dev, SNDCTL_DSP_SPEED, &val) == 0) {
    printf("Current speed: %d\n", val);
  } else {
    warn("ioctl(,SNDCTL_DSP_SPEED,0");
  }
  // Test both standard value set and some "left" values
  doProbeSpeed(96000);
  doProbeSpeed(88200);
  doProbeSpeed(48000);
  doProbeSpeed(44100);
  doProbeSpeed(24000);
  doProbeSpeed(22050);
  doProbeSpeed(16000);
  doProbeSpeed(12000);
  doProbeSpeed(11025);
  doProbeSpeed(8000);
  doProbeSpeed(7800);

  // Print current block size
  if (ioctl(snd_dev, AIOGSIZE, &sndsize) == 0) {
    printf("Current play blocksize: %d\n", sndsize.play_size);
  } else {
    warn("ioctl(,AIOGSIZE)");
  }

  return 0;
}

void
initDevice(void)
{
  int val;
  static const int speed_list[] = {
    48000, 24000, 16000, 8000, 44100, 22050, 11025, 96000,
    0
  };

  if (target == TARGET_FILE) {
    // Use defaults
    snd_dev_speed = 48000;
    return;
  }

  // Initialize output device and select speed among preferred values
  val = 1;
  if (ioctl(snd_dev, SNDCTL_DSP_RESET, &val) < 0)
    err(1, "SNDCTL_DSP_RESET");
  // Required order is: format, channels, speed
  val = AFMT_S16_NE;
  if (ioctl(snd_dev, SNDCTL_DSP_SETFMT, &val) < 0)
    err(1, "SNDCTL_DSP_SETFMT");
  if (val != AFMT_S16_NE)
    errx(1, "fatal: device does not support AFMT_S16_NE");
  val = 1;
  if (ioctl(snd_dev, SNDCTL_DSP_CHANNELS, &val) < 0)
    err(1, "SNDCTL_DSP_CHANNELS");
  if (val != 1)
    errx(1, "fatal: device does not support mono");
  do {
    int i, ret;
    for (i = 0; (val = speed_list[i]) != 0; ++i) {
      ret = setSpeed(val, NULL, NULL);
      if (ret == 1)
        goto speed_is_set;
    }
    for (i = 0; (val = speed_list[i]) != 0; ++i) {
      ret = setSpeed(val, NULL, NULL);
      if (ret > 0)
        goto speed_is_set;
    }
    errx(1, "cannot select speed");
  } while(0);
speed_is_set:
  snd_dev_speed = val;

}

void writeFrames(const uint16_t *wt, size_t wtsize)
{
  const char* buf = (const char*) wt;
  size_t pos, rest;
  ssize_t written;
  pos = 0;
  rest = sizeof(wt[0]) * wtsize;
  while (rest > 0) {
    written = write(snd_dev, &buf[pos], rest);
    if (written <= 0)
      err(1, "write");
    pos += written;
    rest -= written;
  }
}

// vim:ts=2:sts=2:sw=2:et:si:
