PROG=	sss
SRCS=	sss.c

$(PROG): $(SRCS)
	gcc -o $(PROG) $(SRCS) -O0 -Wall -W -g -lsctp

clean:
	rm -f $(PROG)

.PHONY: clean
