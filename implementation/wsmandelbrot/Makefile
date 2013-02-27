CC=gcc
CFLAGS=-I. -lm -lpthread
DEPS = mandelbrot.h deque.h
OBJ = mandelbrot.o deque.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wsmandelbrot: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ core

