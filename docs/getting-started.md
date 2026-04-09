# Getting started

This page takes you from a fresh checkout to a working program. No prior
Babel knowledge assumed — just a C compiler and a terminal.

## 1. Build the compiler

Babel is a single C file. Any C99 compiler will do.

```
$ make
```

That produces a `babel` executable in the project root. There are no
dependencies beyond the C standard library and `libm`.

If you want to build it by hand:

```
$ cc -std=c99 -O2 -o babel src/babel.c -lm
```

To clean up:

```
$ make clean
```

## 2. Run your first program

Create a file called `hello.babel`:

```babel
Say "Hello, world.".
```

Run it:

```
$ ./babel hello.babel
Hello, world.
```

That's the whole cycle: write a file, hand it to `./babel`.

## 3. A slightly bigger program

Save this as `scratch.babel`:

```babel
Let there be a number called "x" that equals 7.
Let there be a number called "y" that equals 6.

Print x times y.

For every i from 1 to 5, do the following:
    Print i times i.
```

Run it:

```
$ ./babel scratch.babel
42
1
4
9
16
25
```

A few things worth noticing in that program:

- Every statement ends with a period.
- Blocks are introduced by a line ending in a colon, and continue for as
  long as the indentation stays deeper than the line that opened them.
- Variables are declared once, with `Let there be a TYPE called "NAME"`.
  The name goes in double quotes.
- The `For every` loop introduces its own counter, `i`, which is only
  visible inside the loop.

## 4. The REPL

Run `./babel` with no arguments and you get an interactive session. Type a
statement, hit enter, see the answer.

```
$ ./babel
> Let there be a number called "x" that equals 10.
> Print x times x.
100
> For every i from 1 to 3, do the following:
...     Print i.
1
2
3
>
```

The REPL recognizes an unfinished block (a line ending in `:`) and prompts
with `...` until you enter an empty line.

To exit: `Ctrl-D` (or `Ctrl-C`).

## 5. The friendly front-end

If you can't remember the exact Babel phrasing, run the **Interpreter of
Tongues** in front of the compiler with `./babel -i` (or `make friendly`).
She accepts freeform English and translates it into Babel before handing
the result to the tower.

```
$ ./babel -i
you> let step be zero, while step is less than ten do the following
     say step and set step to step plus one, say all done

  I think you mean:

    Let there be a number called "step" that equals 0.
    While step is less than 10, do the following:
        Say step.
        Set step to step plus 1.
    Say "All Done".

  correct? yes
  0
  1
  ...
  9
  All Done
```

Behind the friendly prompt is a chain of deterministic passes. Each one
handles a single kind of looseness, so you can trust the result even when
the Interpreter doesn't stop to ask:

- **Shorthand declarations.** `let X equal V`, `let X be V`, and `let X = V`
  all expand to `Let there be a <kind> called "X" that equals V`. The kind
  is inferred from the value: `true`/`false` → `truth`, a quoted string →
  `word`, `empty` → `list`, otherwise → `number`.
- **Case normalization.** Babel identifiers are case-sensitive, but people
  speak without caring. She lowercases everything outside quotes so `N` and
  `n` resolve to the same name. Before lowercasing, she auto-quotes any run
  of two or more Capitalized Words (`Lift Off`, `New York`) as a string
  literal so the traveler's emphasis survives.
- **Context-aware `say`/`print` quoting.** She tracks which names have been
  declared (from `called "X"` and loop variables in `for every X in/from`).
  When you say `say count`, `count` is a known variable and stays as an
  expression; when you say `say all done`, neither word is known, so the
  whole run is wrapped as `"All Done"`.
- **`if X then Y` → `if X, Y`.** Travelers naturally say "if is_prime then
  remember i as primes". She rewrites `then` to the comma Babel expects,
  and if the condition is a bareword truth she quietly completes it to
  `is true`.
- **Block punctuation.** She inserts the comma before `do the following`
  and the colon after, and reflows the body onto its own indented lines.
  A final `Say`/`Print` at the end of a program auto-dedents to the top
  level — the traveler's concluding announcement is almost never meant to
  live inside the last loop.
- **Paragraph-break dedents.** In flat prose, a blank line pops one level
  of nesting and two blank lines pop two. That's how you carve nested
  block structure out of what would otherwise be one long sentence.

The complete table of loose phrases she recognizes lives in `src/babel.c`
under `the_book_of_tongues` — *show me*, *make a*, *loop through*, *for
each*, *whenever*, *or else*, *bigger than*, *same as*, and several dozen
others.

You can also hand her a whole file:

```
$ ./babel -i my-loose-program.babel
```

She pre-processes it into strict Babel, then runs it.

If you'd rather watch the translation happen visually, there's a browser
dashboard in `babel-dashboard/` that wires the Interpreter up to your
microphone and four live panels. See
[`babel-dashboard/README.md`](../babel-dashboard/README.md).

## 6. Running programs other ways

Beyond the default interpreter, a Babel program can also be compiled to a
native binary through C, or transcribed into Python. Both paths are
documented in the [Language reference](language.md#command-line) — but the
default interpreter is the recommended way to run things while you're
learning.

## 7. What to read next

- If you want a systematic tour of the grammar: **[Language reference](language.md)**.
- If you want to write something real and want pattern recipes:
  **[Cookbook](cookbook.md)**.
- If something isn't working and you suspect the language just doesn't
  support it: **[Gotchas and limits](gotchas.md)**.
- If you want to see real, working programs: the `examples/` directory
  has fifteen of them, from `hello.babel` up through `roman_numerals.babel`.
