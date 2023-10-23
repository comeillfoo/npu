CC=gcc
LD=gcc

CFLAGS=-std=gnu11 -Wall -pedantic

TOOLS=pad0

all: $(TOOLS)

pad0: pad0.c
	$(CC) $(CFLAGS) $^ -o $@

pad0_clean:
	rm -f pad0


.PHONY: clean pad0_clean


clean: pad0_clean
