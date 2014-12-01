.if $(MACHINE) == "i386"
#- CFLAGS=-march=i686 -mtune=k8 -mmmx -msse -mfpmath=sse,387
CFLAGS=-march=i586
.endif

CFLAGS += -Os

PROG=	spc
SRCS=	spc.c
NO_MAN=	nefig

.include <bsd.prog.mk>
