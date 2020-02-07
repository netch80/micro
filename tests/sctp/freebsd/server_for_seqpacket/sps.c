#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

static void
server(int s)
{
    char host[200];
    char service[100];
    ssize_t got;
    struct sockaddr_in sia;
    socklen_t slen;
    struct sctp_sndrcvinfo sinfo1;
    char msg[1000];

    slen = sizeof(sia);
    memset(&sinfo1, 0, sizeof(sinfo1));
    got = sctp_recvmsg(s, msg, sizeof(msg),
            (struct sockaddr *) &sia, &slen, &sinfo1, 0);
    if (got < 0) {
        warn("sctp_recvmsg");
        return;
    }
    printf("--->\n");
    printf("Got %zd bytes message\n", got);
    int gni = getnameinfo((struct sockaddr*) &sia, slen,
            host, sizeof(host),
            service, sizeof(service), NI_NUMERICHOST|NI_NUMERICSERV);
    if (gni == 0) {
        printf("The message was from %s:%s\n", host, service);
    }
    else {
        printf("Address error: %s\n", gai_strerror(gni));
        return;
    }
    printf("Parameters: flags=0x%x assoc_id=%u stream=%u ppid=%u\n",
            (unsigned) sinfo1.sinfo_flags,
            (unsigned) sinfo1.sinfo_assoc_id,
            (unsigned) sinfo1.sinfo_stream,
            (unsigned) ntohl(sinfo1.sinfo_ppid));
    struct sctp_sndrcvinfo sinfo;
    // Send greeting
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.sinfo_stream = sinfo1.sinfo_stream;
    if (sctp_sendmsg(s, "hi", 2,
            (struct sockaddr*) &sia, sizeof(sia),
            sinfo1.sinfo_ppid,
            0,
            sinfo1.sinfo_stream,
            0, 0) < 0)
    {
        warn("sctp_sendmsg");
        return;
    }
#if 0
    if (sctp_sendx(s, "hi", 2,
            (struct sockaddr*) &sia, sizeof(sia), &sinfo, 0) < 0)
    {
        warn("sctp_sendx");
        return;
    }
#endif
}

int
main()
{
    struct sockaddr_in sia;
    int ss = -1;

    ss = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if (ss < 0)
        err(1, "socket(SCTP)");
    memset(&sia, 0, sizeof(sia));
    sia.sin_family = AF_INET;
    sia.sin_addr.s_addr = htonl(0x7F000001);
    sia.sin_port = htons(5210);
    if (bind(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0)
        err(1, "bind()");
    if (listen(ss, 1) < 0)
        err(1, "listen()");
    printf("Started to listen, ss=%d\n", ss);
    for(;;) {
        server(ss);
    }
    // UNREACHED
    return 0;
}

// vim:ts=4:sts=2:sw=2:et:si:
