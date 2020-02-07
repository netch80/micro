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
      if (n < psize)
        putc(isprint(c) ? c : '.', stdout);
    }
    printf("|\n");
  }
}

int
main()
{
    struct sockaddr_in sia;
    int ss = -1;
    struct sctp_sndrcvinfo sinfo;
    int rflags;
    char buf[200];
    ssize_t got;

    ss = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (ss < 0)
        err(1, "socket(SCTP)");
    memset(&sia, 0, sizeof(sia));
    sia.sin_family = AF_INET;
    sia.sin_addr.s_addr = htonl(0x7F000001);
    sia.sin_port = htons(5210);
    if (connect(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0)
        err(1, "connect()");
    printf("Connected\n");
    sleep(1);

    memset(&sinfo, 0, sizeof(sinfo));
    got = sctp_recvmsg(ss, buf, sizeof(buf), NULL, 0, &sinfo, &rflags);
    if (got < 0)
        err(1, "recvmsg()");
    printf("Got data for stream %u(%04x): %zd bytes\n",
            (unsigned) sinfo.sinfo_stream, (unsigned) sinfo.sinfo_stream,
            got);
    if (got > 0)
        hexdump(buf, got);

    memset(&sinfo, 0, sizeof(sinfo));
    got = sctp_recvmsg(ss, buf, sizeof(buf), NULL, 0, &sinfo, &rflags);
    if (got < 0)
        err(1, "recvmsg()");
    printf("Got data for stream %u(%04x): %zd bytes\n",
            (unsigned) sinfo.sinfo_stream, (unsigned) sinfo.sinfo_stream,
            got);
    if (got > 0)
        hexdump(buf, got);
#if 0
    if (1 || got == 0) {
      printf("sctp_info contents (%zd bytes):\n", sizeof(sinfo));
      hexdump(&sinfo, sizeof(sinfo));
    }
#endif
    close(ss);
    return 0;
}

// vim:ts=4:sts=2:sw=2:et:si:
