CC=gcc
CCFLAGS=-I. -lm -lpthread -O3

DEPS = mandelbrot.h deque.h worksteal.h rtworksteal.h sequential.h
WSOBJ = deque.o worksteal.o
RTOBJ = rtworksteal.o
SEOBJ = sequential.o

VERS = wsmandelbrot rtmandelbrot semandelbrot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

wsmandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D wsmandel

rtmandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D rtmandel

semandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D semandel


wsmandelbrot: wsmandelobj $(WSOBJ)
	$(CC) -o $@  mandelbrot.o $(WSOBJ) $(CCFLAGS) 

rtmandelbrot: rtmandelobj $(RTOBJ)
	$(CC) -o $@ mandelbrot.o $(RTOBJ) $(CCFLAGS)

semandelbrot: semandelobj $(SEOBJ)
	$(CC) -o $@ mandelbrot.o $(SEOBJ) $(CCFLAGS)

all: $(VERS)

.PHONY: clean

clean:
	rm -f *.o *~ core $(VERS)

