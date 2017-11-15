CC = gcc
CFLAGS = -Wall -g -std=c99 
LDLIBS = -lpthread
OBJS = m1_ProdCons_exo2 m1_ProdCons_exo3 m1_ProdCons_exo4 m1_ProdCons_exo5 m1_ProdCons_base
all: $(OBJS)

$(OBJS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)


clean:
	rm -f $(OBJS)
