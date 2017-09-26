CC = gcc
CFLAGS = -Wall -g
LDLIBS = -fopenmp
OBJS = vigenere vigenere_omp
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm $(OBJS)
