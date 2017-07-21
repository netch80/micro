CFLAGS += -g -Wall -march=i586
CFLAGS += -DHAVE_STRNDUP

PROG = netdbtrap
OBJS = main.o pvector.o

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm -f -- *.o *~ *.core core $(PROG) $(OBJS)
