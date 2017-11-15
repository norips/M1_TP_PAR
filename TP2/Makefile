CC = gcc
CFLAGS = -Wall -g -std=c99 -O3
LDLIBS = -fopenmp
OBJS = vigenere vigenere_omp
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm $(OBJS)
