CC=gcc
CFLAGS=-I. -lm -lpthread -lGL -lGLU -lglut
DEPS = mandelbrot.h deque.h display.h
OBJ = mandelbrot.o deque.o display.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wsmandelbrot: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f *.o *~ core

