CC = gcc
CFLAGS = -Wall -std=gnu11 -DTRACE_SOUHAIT
LDLIBS = -lpthread
OBJS = m1_Voiture_simple m1_Voiture_illimite
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm -f $(OBJS)
