SF_BASE = /home/netch/scmwork/github/ucb-bar/berkeley-softfloat-3
SF_INCDIR = $(SF_BASE)/source/include
SF_LIBDIR = $(SF_BASE)/build/Linux-x86_64-GCC

CC = $(shell which gcc48 2>/dev/null)
ifeq "$(CC)" ""
CC = $(shell which gcc-4.8 2>/dev/null)
endif
ifeq "$(CC)" ""
CC = $(shell which gcc5 2>/dev/null)
endif
CFLAGS = -Og -g -Wall -W
LIBS =
CFLAGS += -I$(SF_INCDIR)
LIBS += $(SF_LIBDIR)/softfloat.a
ifneq "$(USE_FPU)" ""
CFLAGS += -mfpmath=387
else
CFLAGS += -mfpmath=sse
endif
ifneq "$(USE_MEMCOPY)" ""
CFLAGS += -DMEMCOPY
endif

t: t.c
	$(CC) -o t t.c $(CFLAGS) $(LIBS)

clean:
	rm -f t t.o

.PHONY: clean
