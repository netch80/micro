#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <err.h>

int flat_recv = 0;
int own_msg = 0;

void
hexdump(const void *pp, size_t psize)
{
  size_t i, j, n;
  const unsigned char *p = pp;
  for (i = 0; i < psize; i += 16) {
    printf("%08zx:", i);
    for (j = 0, n = i; j < 16; ++n, ++j) {
      if (n < psize)
        printf(" %02x", (int) p[n]);
      else
        printf("   ");
    }
    printf(" |");
    for (j = 0, n = i; j < 16; ++n, ++j) {
      int c = p[n];
      if (n < psize) {
          putc(isprint(c) ? c : '.', stdout);
      }
    }
    printf("|\n");
  }
}

int
main(int argc, char **argv)
{
    struct sockaddr_in sia;
    int rc, i, ss = -1;
    struct sctp_sndrcvinfo sinfo;
    struct sctp_event_subscribe events;
    int rflags;
    char buf[200];
    ssize_t got;

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "flat_send")) {
            flat_recv = 1;
        }
        if (!strcmp(argv[i], "own_msg")) {
            own_msg = 1;
        }
    }

    ss = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (ss < 0) {
        err(1, "socket(SCTP)");
    }
    memset(&sia, 0, sizeof(sia));
    sia.sin_family = AF_INET;
    sia.sin_addr.s_addr = htonl(0x7F000001);
    sia.sin_port = htons(5210);
    memset(&events, 0, sizeof events);
    socklen_t optlen = sizeof events;
    rc = getsockopt(ss, IPPROTO_SCTP, SCTP_EVENTS, &events, &optlen);
    events.sctp_data_io_event = 1;
    rc = setsockopt(ss, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof events);
    if (rc < 0) {
        err(1, "SCTP_EVENTS");
    }
    if (connect(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0) {
        err(1, "connect()");
    }
    printf("Connected\n");
    rflags = 0;
    for(;;) {
        memset(&sinfo, 0, sizeof(sinfo));
        if (flat_recv) {
            got = recv(ss, buf, sizeof buf, 0);
            if (got < 0) {
                err(1, "recvmsg()");
            }
        } else if (own_msg) {
            struct msghdr hdr;
            struct iovec iov;
            char cbuf[CMSG_SPACE(sizeof(sinfo))];

            iov.iov_base = buf;
            iov.iov_len = sizeof buf;
            memset(&hdr, 0, sizeof hdr);
            hdr.msg_iov = &iov;
            hdr.msg_iovlen = 1;
            hdr.msg_control = &cbuf;
            hdr.msg_controllen = sizeof cbuf;
            got = recvmsg(ss, &hdr, 0);
            rflags = hdr.msg_flags;
            if (got < 0) {
                err(1, "recvmsg()");
            }
            printf("After recvmsg: controllen=%zu\n", hdr.msg_controllen);
            struct cmsghdr *cmsg;
            for (cmsg = CMSG_FIRSTHDR(&hdr);
                    cmsg != NULL;
                    cmsg = CMSG_NXTHDR(&hdr, cmsg))
            {
                if (cmsg->cmsg_level == IPPROTO_SCTP &&
                        cmsg->cmsg_type == SCTP_SNDRCV)
                {
                    memmove(&sinfo, CMSG_DATA(cmsg), sizeof sinfo);
                    break;
                }
            }
        } else {
            got = sctp_recvmsg(ss, buf, sizeof buf,
                    NULL, 0,
                    &sinfo, &rflags);
            if (got < 0) {
                err(1, "recvmsg()");
            }
        }
        if (got == 0) {
            printf("Got EOF\n");
            break;
        }
        printf("Got data for stream %u: len=%zd\n flags=%zx\n",
                (unsigned) sinfo.sinfo_stream,
                (ssize_t) got, (ssize_t) rflags);
        printf("Buf (%zd bytes):\n", (ssize_t) got);
        if (got > 0) {
            hexdump(&buf, got);
        }
        if (got >= 0) {
          printf("sctp_sndrcvinfo contents (%zd bytes):\n", sizeof(sinfo));
          hexdump(&sinfo, sizeof(sinfo));
        }
    }
    close(ss);
    return 0;
}

// vim:ts=4:sts=2:sw=2:et:si:
