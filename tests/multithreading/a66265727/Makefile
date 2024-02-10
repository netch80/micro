OBJS = t.o sub1.o
CFLAGS = -Og -g -Wall -W

t: $(OBJS)
	$(CXX) -o t $(OBJS)

%.o: %.cxx
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $<

clean:
	rm -f t $(OBJS)

.PHONY: clean
