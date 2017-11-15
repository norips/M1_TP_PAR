CC = gcc
CFLAGS = -Wall -std=gnu11 -DTRACE_SOUHAIT
LDLIBS = -lpthread
OBJS = exoTp4_q1 exoTp4_q2
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm -f $(OBJS)
