CC      = cc
CFLAGS  = -std=c99 -Wall -Wno-unused-function -O2
LDFLAGS = -lm

all: babel

babel: src/babel.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: babel
	./babel examples/primes.babel

repl: babel
	./babel

clean:
	rm -f babel a.out

.PHONY: all run repl clean
