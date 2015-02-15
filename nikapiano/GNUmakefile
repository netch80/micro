PROG=	nikapiano
SRCS=	main.c spkr.c piano.c
LDADD=	-lm
DEBUG_FLAGS=-g -O0

OBJS=$(patsubst %.c, %.o, $(SRCS))

$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDADD)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG_FLAGS)
.PHONY: clean

clean:
	-rm -f -- $(PROG) $(OBJS)
