CC := gcc
CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS := $(shell pkg-config --libs gtk+-3.0) -lm

all:    rocketeq

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

rocketeq:       app.o
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f *.o rocketeq
