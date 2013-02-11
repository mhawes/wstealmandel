CC=gcc
CFLAGS=-I. -lm -O3
DEPS = mandelbrot.h
OBJ = mandelbrot.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mandelbrot: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ core

