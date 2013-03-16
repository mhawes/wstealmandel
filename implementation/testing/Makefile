CC=gcc
CCFLAGS=-I. -lm -lpthread -O3

DEPS = deque.h
OBJ  = deque.o test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)
	
test: $(OBJ)
	$(CC) -o $@ $(OBJ) $(CCFLAGS)

