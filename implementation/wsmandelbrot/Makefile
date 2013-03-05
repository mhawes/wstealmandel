CC=gcc
CFLAGS=-I. -lm -lpthread -O3
DEPS = mandelbrot.h deque.h worksteal.h
OBJ = mandelbrot.o deque.o worksteal.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wsmandelbrot: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ core

