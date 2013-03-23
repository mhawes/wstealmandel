CC=gcc
CCFLAGS=-I. -lm -lpthread -O3

BINDIR = ./bin/

DEPS = mandelbrot.h deque.h worksteal.h rtworksteal.h sequential.h noscheduling.h
WSOBJ = deque.o worksteal.o
RTOBJ = rtworksteal.o
SEOBJ = sequential.o
NSOBJ = noscheduling.o

VERS = wsmandelbrot rtmandelbrot semandelbrot nsmandelbrot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

all: $(VERS)

wsmandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D wsmandel

rtmandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D rtmandel

semandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D semandel

nsmandelobj: mandelbrot.c $(DEPS)
	$(CC) -c -o mandelbrot.o mandelbrot.c $(CCFLAGS) -D nsmandel


wsmandelbrot: wsmandelobj $(WSOBJ)
	$(CC) -o $(BINDIR)$@ mandelbrot.o $(WSOBJ) $(CCFLAGS) 

rtmandelbrot: rtmandelobj $(RTOBJ)
	$(CC) -o $(BINDIR)$@ mandelbrot.o $(RTOBJ) $(CCFLAGS)

semandelbrot: semandelobj $(SEOBJ)
	$(CC) -o $(BINDIR)$@ mandelbrot.o $(SEOBJ) $(CCFLAGS)

nsmandelbrot: nsmandelobj $(NSOBJ)
	$(CC) -o $(BINDIR)$@ mandelbrot.o $(NSOBJ) $(CCFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ core $(addprefix $(BINDIR),$(VERS))

