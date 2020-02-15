PROG=	nikapiano
OBJS=	main.o spkr.o piano.o alsa.o
LDADD=	-lm -lasound
DEBUG_FLAGS=-g -O0 -Wall -W

CC?=	cc
CXX?=	c++

$(PROG): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDADD)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG_FLAGS)

%.o: %.cxx
	$(CXX) -o $@ -c $< $(CFLAGS) $(DEBUG_FLAGS)

.PHONY: clean

clean:
	-rm -f -- $(PROG) $(OBJS)
