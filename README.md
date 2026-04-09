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

**Speak loosely.** If you'd rather not memorize the exact Babel phrasing, run the Interpreter of Tongues in front of the tower with `./babel -i` (or `make friendly`). She accepts freeform English — `let step be zero`, `show me x`, `make a number`, `whenever`, `loop through`, and a few dozen other everyday phrases — translates them into Babel, shows you what she heard, asks for confirmation on anything she's not sure about, and then hands the result to the tower.

```
$ ./babel -i
you> let step be zero, while step is less than ten do the following say step and set step to step plus one, say all done

  I think you mean:

    Let there be a number called "step" that equals 0.
    While step is less than 10, do the following:
        Say step.
        Set step to step plus 1.
    Say "All Done".

  correct? yes
  0
  1
  2
  ...
  9
  All Done
```

She does quite a bit of deterministic work on your sentence before handing it over:

- **Shorthand declarations.** `let X equal V` / `let X be V` / `let X = V` expand to the full `Let there be a <kind> called "X" that equals V`. The kind is inferred from the value — `true`/`false` → `truth`, a quoted string → `word`, `empty` → `list`, otherwise → `number`.
- **Case normalization.** She lowercases everything outside quotes so `N` and `n` resolve to the same name, and auto-quotes runs of two or more Capitalized Words (`Lift Off`, `New York`) as string literals before lowercasing, so the traveler's capitalization hint survives.
- **Context-aware `say`/`print` quoting.** She tracks which names have been declared (from `called "X"` and loop variables in `for every X in/from`). When you say `say count`, `count` is a known variable and stays as an expression; when you say `say all done`, neither word is known, so the whole run is wrapped as `"All Done"`.
- **`if X then Y` → `if X, Y`.** Travelers naturally say "if is_prime then remember i as primes". She rewrites `then` to the comma Babel expects, and if the condition is a bareword truth (`if is_prime`), she quietly completes it to `if is_prime is true`.
- **Block punctuation.** She inserts the comma before `do the following` and the colon after, and reflows the body onto its own indented lines. A final `Say`/`Print` at the end of a program auto-dedents to the top level — the traveler's concluding announcement was almost never meant to live inside the last loop.
- **Paragraph-break dedents.** A blank line in the input pops one level of nesting. Two blank lines pop two levels. That's how flat prose carves out nested block structure.

**Watch the translation happen.** There's also a living browser dashboard in `babel-dashboard/` — four panels (voice, prose, translation, output) and a conversation overlay, with live speech-to-Babel translation as you talk or type. See [`babel-dashboard/README.md`](babel-dashboard/README.md) to run it.

## About the source

The compiler is a single C file, `src/babel.c`. It's written so that the code and the comments together read as a short story about four builders — a Lexer, a Parser, an Evaluator, and a Scribe — raising a tower out of words. The identifiers follow the story, the comments narrate it, and the error messages are written in the same voice.

It still compiles with plain `cc`. It's just that when you open it, you get a story as well as a compiler. `STORY.md` has the same story with the C stripped out, if you just want to read the prose.

This is a stylistic experiment, not a claim that literate programming is the right way to write compilers. I wrote it this way because I wanted to, and because it forced me to think harder about naming.

## Documentation

Full docs live in the `docs/` folder:

- [`docs/getting-started.md`](docs/getting-started.md) — build, run, first program, REPL.
- [`docs/language.md`](docs/language.md) — the complete language reference.
- [`docs/cookbook.md`](docs/cookbook.md) — short idiomatic snippets.
- [`docs/gotchas.md`](docs/gotchas.md) — limitations and rough edges.

## Examples

- `examples/hello.babel` — hello world
- `examples/greeting.babel` — word variables and concatenation
- `examples/fizzbuzz.babel` — fizzbuzz
- `examples/countdown.babel` — a `While` loop counting down to liftoff
- `examples/fibonacci.babel` — recursive fibonacci
- `examples/factorial.babel` — recursive factorial
- `examples/primes.babel` — the sieve above
- `examples/primality.babel` — a recipe that returns a truth
- `examples/average.babel` — the sum and length of a list
- `examples/walkabout.babel` — walking a list with `For every x in LIST`
- `examples/multiplication_table.babel` — nested loops that build a grid
- `examples/gcd.babel` — Euclid's algorithm as a recursive recipe
- `examples/power.babel` — recursive exponentiation
- `examples/collatz.babel` — a `While` loop tracing the 3n+1 sequence
- `examples/perfect_numbers.babel` — recipes and loops finding perfect numbers
- `examples/happy_numbers.babel` — digit arithmetic inside a nested recipe
- `examples/leap_years.babel` — the Gregorian rule expressed as nested ifs
- `examples/triangle.babel` — building up strings row by row
- `examples/ackermann.babel` — a small witness to deep recursion
- `examples/roman_numerals.babel` — a longer recipe converting numbers to Roman
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
