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
server(int s, struct sockaddr* sa, socklen_t sl)
{
    char host[200];
    char service[100];
    ssize_t sent;
    int gni = getnameinfo(sa, sl, host, sizeof(host),
            service, sizeof(service), NI_NUMERICHOST|NI_NUMERICSERV);
    if (gni == 0) {
        printf("\nConnect from %s:%s\n", host, service);
    }
    else {
        printf("Address error: %s\n", gai_strerror(gni));
        close(s);
        return;
    }
    struct sctp_sndrcvinfo sinfo;
    // Send greeting
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.sinfo_stream = 1 + (random() % 10);
    printf("Sending for stream %d\n", (int) sinfo.sinfo_stream);
    fflush(stdout);
    sent = sctp_send(s, "hi", 2, &sinfo, 0);
    if (sent < 0) {
        warn("sctp_send");
        close(s);
        return;
    }
    if (sent != 2) {
        warnx("sctp_send(): bad sent count (case 1)");
        close(s);
        return;
    }
    sent = sctp_send(s, " world", 6, &sinfo, 0);
    if (sent < 0) {
        warn("sctp_send");
        close(s);
        return;
    }
    if (sent != 6) {
        warnx("sctp_send(): bad sent count (case 2)");
        close(s);
        return;
    }
    // XXX
    close(s);
}

int
main()
{
    struct sockaddr_in sia;
    int ss = -1;

    ss = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (ss < 0)
        err(1, "socket(SCTP)");
    memset(&sia, 0, sizeof(sia));
    sia.sin_family = AF_INET;
    sia.sin_addr.s_addr = htonl(0x7F000001);
    sia.sin_addr.s_addr = 0;
    sia.sin_port = htons(5210);
    if (bind(ss, (struct sockaddr*)&sia, sizeof(sia)) < 0)
        err(1, "bind()");
    if (listen(ss, 1) < 0)
        err(1, "listen()");
    printf("Started to listen, ss=%d\n", ss);
    for(;;) {
        socklen_t sl;
        int sconn;
        sl = sizeof(sia);
        sconn = accept(ss, (struct sockaddr*)&sia, &sl);
        if (sconn < 0) {
            warn("accept()");
            usleep(20000);
            continue;
        }
        server(sconn, (struct sockaddr*)&sia, sl);
    }
    // UNREACHED
    return 0;
}

// vim:ts=4:sts=2:sw=2:et:si:
