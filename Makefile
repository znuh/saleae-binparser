OBJS=example_main.o saleae_v2_digital.o mmap.o

CFLAGS=-Wall -O3

all: example

example: $(OBJS)
	$(CC) $(CFLAGS) -o example $(OBJS)

clean:
	rm -f example $(OBJS)


