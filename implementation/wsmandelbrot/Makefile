CC=gcc
CCFLAGS=-I. -lm -lpthread -g

DEPS = mandelbrot.h deque.h worksteal.h rtworksteal.h
WSOBJ = mandelbrot.o deque.o worksteal.o
RTOBJ = mandelbrot.o rtworksteal.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

wsmandelbrot: $(WSOBJ)
	gcc -o $@ $^ $(CCFLAGS) -D wsmandel

rtmandelbrot: $(RTOBJ)
	gcc -o $@ $^ $(CCFLAGS) -D rtmandel

.PHONY: clean

clean:
	rm -f *.o *~ core

