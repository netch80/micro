OBJS = cpuid.o
CFLAGS += -Wall

cpuid: cpuid.o
	$(CC) -o $@ $(OBJS)

.c.o:
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o cpuid

.PHONY: clean
