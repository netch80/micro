#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <err.h>

#define PRECONNECT

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
      if (n < psize) {
        int c = p[n];
        putc(isprint(c) ? c : '.', stdout);
      }
    }
    printf("|\n");
  }
}

int
main()
{
    struct sockaddr_in sia, sia2;
    socklen_t slen;
    int ss = -1;
    struct sctp_sndrcvinfo sinfo;
    int rflags;
    char buf[200];
    ssize_t sent, got;

    srandomdev();
    //- ss = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    ss = socket(AF_INET, SOCK_SEQPACKET, 0);
    if (ss < 0)
        err(1, "socket(SCTP)");
    memset(&sia, 0, sizeof(sia));
    sia.sin_family = AF_INET;
    sia.sin_addr.s_addr = htonl(0x7F000001);
    sia.sin_port = 0;
    if (bind(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0)
        err(1, "bind()");
    slen = sizeof(sia2);
    if (getsockname(ss, (struct sockaddr*)&sia2, &slen) < 0)
        err(1, "getsockname()");
    printf("Bound to port %d\n", ntohs(sia2.sin_port));
    sia.sin_port = htons(5210);
#if defined(PRECONNECT)
    if (connect(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0)
        err(1, "connect()");
    printf("Connected\n");
#endif
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.sinfo_ppid = 42;
    sinfo.sinfo_stream = random() % 10;
    //- sinfo.sinfo_flags |= SCTP_UNORDERED;
    printf("Sending for stream %u\n", (unsigned) sinfo.sinfo_stream);
#if defined(PRECONNECT)
    //- sent = sctp_send(ss, "preved", 6,
    //-         &sinfo, 0);
    sent = sctp_sendmsg(ss, "preved", 6,
            (struct sockaddr*) &sia, sizeof(sia),
            htonl(42),                  // ppid
            0,                          // flags
            sinfo.sinfo_stream,         // stream number
            0,                          // TTL
            0                           // context
    );
#else
    sent = sctp_sendx(ss, "preved", 6,
            (struct sockaddr*) &sia, 1,
            &sinfo, 0);
#endif
    if (sent < 0)
        err(1, "sctp_send()");
    alarm(5);
    got = sctp_recvmsg(ss, buf, sizeof(buf), NULL, 0, &sinfo, &rflags);
    if (got < 0)
        err(1, "recvmsg()");
    printf("Got data for stream %u(%04x): %zd bytes\n",
            (unsigned) sinfo.sinfo_stream, (unsigned) sinfo.sinfo_stream,
            got);
    if (got > 0)
        hexdump(buf, got);
    if (got == 0) {
        printf("sctp_info contents (%zd bytes):\n", sizeof(sinfo));
        hexdump(&sinfo, sizeof(sinfo));
    }
    close(ss);
    return 0;
}

// vim:ts=4:sts=2:sw=2:et:si:
