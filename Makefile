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

# Start a REPL that runs the Interpreter of Tongues in front of
# the tower — you can speak however you like and she will
# translate for the parser.
friendly: babel
	./babel -i

clean:
	rm -f babel a.out

.PHONY: all run repl friendly clean
