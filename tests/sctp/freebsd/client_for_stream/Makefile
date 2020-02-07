.if $(MACHINE) == "i386"
CFLAGS=-march=i686 -mtune=k8 -mmmx -msse -mfpmath=sse,387
.endif

PROG=	ssc
SRCS=	ssc.c
NO_MAN=	nefig

.include <bsd.prog.mk>
