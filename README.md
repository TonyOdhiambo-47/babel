# Babel

A programming language you already know how to read.

```babel
Let there be a list called "primes" that begins empty.

For every candidate from 2 to 100, do the following:
    Let there be a truth called "looks prime" that equals true.
    For every tester from 2 to candidate minus 1, do the following:
        If candidate is divisible by tester, set "looks prime" to false.
    If looks prime equals true, remember candidate as primes.

Print primes, separated by ", ".
```

```
$ make
$ ./babel examples/primes.babel
2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
```

Or start a conversation:

```
$ ./babel
```

Or compile a Babel program to a native binary at the speed of `cc -O3`:

```
$ ./babel -c -o primes examples/primes.babel
$ ./primes
```

Or transcribe the same program into Python, so you can read it in
a language you already know:

```
$ ./babel -p -o primes.py examples/primes.babel
$ python3 primes.py
```

## What happens when you open `src/babel.c`

Read it.

The whole compiler — lexer, parser, tree-walking evaluator, C
code generator, Python transcriber, REPL — is written as a
single, continuous short story about builders raising a tower
out of words. Every identifier, every comment, every error
message is part of the narrative. You can read it as a fable or
you can read it as a working compiler. Both readings are
correct. That was the point of the tower all along.

The same story, without the C syntax, is in `STORY.md`.

## Examples

- `examples/hello.babel` — the smallest sentence
- `examples/fizzbuzz.babel` — the oldest interview question
- `examples/fibonacci.babel` — recursion
- `examples/factorial.babel` — recursion, the friendly kind
- `examples/primes.babel` — a list, a loop, a sieve
- `examples/poetry.babel` — a program that is a poem that counts primes

## Tests

```
$ tests/run_tests.sh
```

Runs a small choir of unit programs through the evaluator, then
puts the same examples through the Python transcription and
checks both say the same thing. The tower should pass all of
them.

## Benchmarks

```
$ bench/run.sh
```

Runs the same prime-counting program through four runners —
the tree walker, the native C backend, the Python transcription,
and a hand-written Python equivalent — and reports their times.

## Author

Tony Odhiambo
