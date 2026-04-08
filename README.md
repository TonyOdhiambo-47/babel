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

## What happens when you open `src/babel.c`

Read it.

## Examples

- `examples/hello.babel` — the smallest sentence
- `examples/fizzbuzz.babel` — the oldest interview question
- `examples/fibonacci.babel` — recursion
- `examples/factorial.babel` — recursion, the friendly kind
- `examples/primes.babel` — a list, a loop, a sieve
- `examples/poetry.babel` — a program that is a poem that counts primes

## Author

Tony Odhiambo
