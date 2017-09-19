CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lpthread
OBJS = 11_thd_afficher 12_thd_afficher 13_thd_afficher
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm $(OBJS)
