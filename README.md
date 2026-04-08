# Babel

Babel is a small language where the source code is structured English. Here's a program that prints the primes below 100:

```babel
Let there be a list called "primes" that begins empty.

For every candidate from 2 to 100, do the following:
    Let there be a truth called "looks prime" that equals true.
    For every tester from 2 to candidate minus 1, do the following:
        If candidate is divisible by tester, set "looks prime" to false.
    If looks prime equals true, remember candidate as primes.

Print primes, separated by ", ".
```

Build and run it:

```
$ make
$ ./babel examples/primes.babel
2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
```

## Running programs

There are three ways to run a Babel program.

**Interpret it directly** — the default. Fastest to try, slowest to run.

```
$ ./babel examples/primes.babel
```

**Compile it to a native binary** via C. Babel parses your program, emits equivalent C, and hands it to `cc -O3`. The result runs at the speed of compiled C.

```
$ ./babel -c -o primes examples/primes.babel
$ ./primes
```

**Transcribe it to Python.** Useful if you want to read what a Babel program actually does in a language you already know, or drop it into a larger Python project.

```
$ ./babel -p -o primes.py examples/primes.babel
$ python3 primes.py
```

**Start a REPL** by running `./babel` with no arguments.

## About the source

The compiler is a single C file, `src/babel.c`. It's written so that the code and the comments together read as a short story about four builders — a Lexer, a Parser, an Evaluator, and a Scribe — raising a tower out of words. The identifiers follow the story, the comments narrate it, and the error messages are written in the same voice.

It still compiles with plain `cc`. It's just that when you open it, you get a story as well as a compiler. `STORY.md` has the same story with the C stripped out, if you just want to read the prose.

This is a stylistic experiment, not a claim that literate programming is the right way to write compilers. I wrote it this way because I wanted to, and because it forced me to think harder about naming.

## Examples

- `examples/hello.babel` — hello world
- `examples/fizzbuzz.babel` — fizzbuzz
- `examples/fibonacci.babel` — recursive fibonacci
- `examples/factorial.babel` — recursive factorial
- `examples/primes.babel` — the sieve above
- `examples/poetry.babel` — a program whose source reads as a poem

## Tests

```
$ tests/run_tests.sh
```

Runs a set of short programs through the interpreter and through the Python transcription, and checks that both produce the same output.

## Benchmarks

```
$ bench/run.sh
```

Runs the same prime-counting program through four backends — the tree-walking interpreter, the native C binary, the Python transcription, and a hand-written Python equivalent — and prints how long each one took.

## Author

Tony Odhiambo
