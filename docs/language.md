# Language reference

This is the complete set of rules for writing Babel. It's organized
bottom-up: first lexical matters (what a single token is), then types,
then expressions, then statements, then the command-line interface.

## Contents

1. [Lexical structure](#lexical-structure)
2. [Types](#types)
3. [Declarations](#declarations)
4. [Assignment](#assignment)
5. [Expressions](#expressions)
6. [Conditions and comparisons](#conditions-and-comparisons)
7. [Printing](#printing)
8. [Conditionals](#conditionals)
9. [Loops](#loops)
10. [Lists](#lists)
11. [Recipes (functions)](#recipes-functions)
12. [Pronouns](#pronouns)
13. [The command line](#command-line)

---

## Lexical structure

### Words, punctuation, whitespace

A Babel program is a sequence of words, numbers, strings, and small marks
of punctuation (`. , :`). Spaces and tabs between words don't matter.

Every **statement** ends with a period (`.`). Every **block** is opened by
a line ending in a colon (`:`) and continued by indentation.

### Indentation

Blocks are delimited by indentation, the way they are in Python. One
leading space counts as one level of depth; a tab counts as four.

```babel
For every i from 1 to 3, do the following:
    Print i.
    For every j from 1 to 2, do the following:
        Print j.
```

The important rule: the *inner* block must be more deeply indented than
the line that opened it. How much more doesn't matter, as long as it's
consistent within the block.

### Comments

A **comment** is a line whose first non-whitespace character is `#`. The
whole line is ignored.

```babel
# This is a comment.

    # So is this — indentation doesn't matter for comments.
Print "hello".
```

There are **no inline comments**. You cannot put `# ...` at the end of a
statement. A `#` inside a statement will be treated as a stray character.

### Identifiers (names)

A name starts with a letter or underscore, and may contain letters,
digits, underscores, and apostrophes. Names are case-sensitive.

Valid: `x`, `total`, `sum_of_divisors`, `line2`, `tony's_counter`.

Invalid as a name: `2cool` (starts with a digit), `hello-world` (hyphen).

When you introduce a name with `Let there be...`, you write the name
**in double quotes**. Once introduced, you refer to it without quotes.

```babel
Let there be a number called "score" that equals 10.
Print score.
```

Names that are already Babel keywords (`a`, `the`, `is`, `to`, `for`,
`while`, etc.) cannot be used as identifiers. See the [list of keywords
below](#reserved-words).

### String literals

A string literal is wrapped in double quotes: `"hello"`. It must open
and close on the same line — Babel has no multi-line strings, and no
escape sequences. To get a double quote inside a string, you can't.

Strings are stored in a fixed-size buffer of about 80 characters. Longer
strings get truncated. Plan accordingly.

### Number literals

A number is one or more digits, optionally followed by a decimal point
and more digits: `0`, `42`, `3.14`, `100`. There are **no negative
literals** — write `0 minus 5` if you need negative five.

Internally, numbers are double-precision floats. They print as integers
when they happen to be whole (`42`, not `42.0`).

### Reserved words

The following words are part of the grammar and cannot be used as
identifiers. Most are also recognized with an initial capital letter
(`Let`, `If`, `For`, etc.) — the language is case-insensitive for
keywords:

```
let there be a an called that begins equals
for every from to do the following while
if otherwise is not indeed
print say number list truth word
remember as set change stop
plus minus times divided by modulo
greater less than divisible contains
empty length of sum separated
answer give back and with
it our true false nothing
```

---

## Types

Babel has four main types. Each has a keyword you use to declare a
variable of that type.

| Keyword  | Meaning                            | Example value       |
|----------|------------------------------------|---------------------|
| `number` | A double-precision floating-point  | `7`, `3.14`         |
| `word`   | A short string (≤ ~80 chars)       | `"hello"`           |
| `truth`  | A boolean                          | `true`, `false`     |
| `list`   | An ordered, growable collection    | `[1, 2, 3]`         |

There is also `nothing`, the empty value. You rarely declare it, but it's
what an uninitialized variable, an empty `Print`, or a recipe that never
gave an answer evaluates to.

---

## Declarations

Every name must be introduced with a `Let there be` sentence before it's
used.

```babel
Let there be a TYPE called "NAME".
Let there be a TYPE called "NAME" that equals EXPR.
Let there be a TYPE called "NAME" that begins EXPR.
Let there be a list called "NAME" that begins empty.
```

The `that equals` and `that begins` clauses are interchangeable —
`that equals` reads better for numbers and words; `that begins` reads
better for lists.

Defaults if you don't give an initial value:

- `number` → `0`
- `word`   → `""`
- `truth`  → `false`
- `list`   → an empty list

Examples:

```babel
Let there be a number called "score" that equals 0.
Let there be a word called "name" that equals "world".
Let there be a truth called "done" that equals false.
Let there be a list called "primes" that begins empty.
Let there be a list called "errors" that begins empty.
```

---

## Assignment

Once a name exists, you change its value with `Set` or `Change`. The two
words are synonyms.

```babel
Set NAME to EXPR.
Change NAME to EXPR.
```

The name can optionally be in double quotes, which is useful if it has
spaces (names with spaces are allowed if you introduced them that way).

```babel
Let there be a number called "score" that equals 0.
Set score to 10.
Change score to score plus 1.
Set "score" to 42.
```

Assignment **mutates** in place. A recipe can assign to a variable
declared in an outer scope (see [Recipes](#recipes-functions)).

---

## Expressions

An expression is anything that evaluates to a value: a literal, a name,
an arithmetic combination, a recipe call, or one of the built-in
measures below.

### Literals

```babel
42            # a number
3.14          # a number
"hello"       # a word
true          # a truth
false         # a truth
nothing       # the empty value
```

### Arithmetic

Babel writes the operators as English words:

| Operator     | Meaning         |
|--------------|-----------------|
| `plus`       | addition        |
| `minus`      | subtraction     |
| `times`      | multiplication  |
| `divided by` | division        |
| `modulo`     | remainder       |

Precedence works the way you'd expect: `times`, `divided by`, and
`modulo` bind tighter than `plus` and `minus`. Everything is
left-associative.

```babel
Print 2 plus 3 times 4.        # 14, not 20
Print 10 minus 6 divided by 2. # 7, not 2
```

### No parentheses

Babel has **no parenthesized expressions**. If you need to force an
unusual grouping, use an intermediate variable:

```babel
# We want (x minus 1) divided by 2, not x minus (1 divided by 2):
Let there be a number called "shifted" that equals x minus 1.
Let there be a number called "half" that equals shifted divided by 2.
```

### String concatenation

`plus` also concatenates words, as long as the **left-hand side** is a
word:

```babel
Let there be a word called "hi" that equals "Hello, ".
Let there be a word called "who" that equals "world".
Say hi plus who plus "!".      # Hello, world!
```

You cannot mix words and numbers with `plus`. Convert the number to a
word first — which Babel doesn't yet have a built-in for, so in practice,
keep your concatenations word-to-word.

### The length of / the sum of

Two built-in aggregate expressions work on lists:

```babel
the length of NAME     # how many elements, as a number
the sum of NAME        # the sum of all numbers in the list
```

```babel
Let there be a list called "xs" that begins empty.
Remember 1 as xs.
Remember 2 as xs.
Remember 3 as xs.
Print the length of xs.    # 3
Print the sum of xs.       # 6
```

### Membership

The `contains` operator asks whether a list contains a value:

```babel
If xs contains 2, say "yes".
```

Note: `contains` is supported by the default interpreter. The native C
backend and the Python transcriber do not yet handle it.

---

## Conditions and comparisons

A condition is a single comparison between two expressions. Babel knows
the following comparators:

| Phrase                       | Meaning        |
|------------------------------|----------------|
| `X equals Y`                 | equality       |
| `X is Y`                     | equality       |
| `X is not Y`                 | inequality     |
| `X is greater than Y`        | `>`            |
| `X is less than Y`           | `<`            |
| `X is divisible by Y`        | `X mod Y == 0` |
| `X is not divisible by Y`    | `X mod Y != 0` |
| `LIST contains X`            | membership     |

That's the whole set. There is **no `>=` or `<=`**, and there is **no
`and` / `or`** for combining conditions. If you need a compound
condition, nest `If` statements or use `Otherwise, if ...`:

```babel
If year is divisible by 400, remember year as leaps.
Otherwise, if year is not divisible by 100, if year is divisible by 4, remember year as leaps.
```

---

## Printing

Babel has two ways to speak aloud:

```babel
Print EXPR.
Print EXPR, separated by EXPR.
Say EXPR.
```

Both print a value and follow it with a newline. `Print` can take a
separator, which is used when the value is a list:

```babel
Let there be a list called "xs" that begins empty.
Remember 1 as xs.
Remember 2 as xs.
Remember 3 as xs.

Print xs.                         # 1, 2, 3      (default separator)
Print xs, separated by " | ".     # 1 | 2 | 3
Print xs, separated by "".        # 123
```

`Say` is the simpler sibling — it prints a single value with no
separator handling. In practice use `Say` for single strings and `Print`
when you want fine control over how a list is formatted.

---

## Conditionals

```babel
If CONDITION, STATEMENT.
If CONDITION, STATEMENT.
Otherwise, STATEMENT.
```

The `Otherwise` clause is optional, and it always attaches to the
immediately preceding `If`. The statement after a condition is a **single
statement** — Babel has no `{ ... }`-style block form for `If`. If you
need multiple actions in a branch, use a recipe, or use nested structures.

```babel
If x is greater than 0, print "positive".
Otherwise, print "zero or negative".
```

Chained cases are written with `Otherwise, if ...`:

```babel
If n is divisible by 15, say "FizzBuzz".
Otherwise, if n is divisible by 3, say "Fizz".
Otherwise, if n is divisible by 5, say "Buzz".
Otherwise, say n.
```

---

## Loops

### Counted loop

```babel
For every COUNTER from EXPR to EXPR, do the following:
    BODY
```

- `COUNTER` is introduced by the loop; you do not `Let there be` it
  first.
- The loop is **inclusive** at both ends: `from 1 to 5` runs with the
  counter taking the values 1, 2, 3, 4, 5.
- The counter is only visible inside the loop.

```babel
For every i from 1 to 5, do the following:
    Print i.
```

Nested loops are fine:

```babel
For every row from 1 to 3, do the following:
    For every col from 1 to 3, do the following:
        Print row times col.
```

### While loop

```babel
While CONDITION, do the following:
    BODY
```

```babel
Let there be a number called "n" that equals 10.
While n is greater than 0, do the following:
    Print n.
    Set n to n minus 1.
```

### Breaking out with Stop

`Stop.` breaks out of the **innermost** enclosing loop.

```babel
For every i from 1 to 100, do the following:
    If i equals 7, stop.
    Print i.
```

There is no `continue`. To skip an iteration, wrap the iteration body in
`If ..., do-the-work`.

---

## Lists

Lists are declared like any other variable:

```babel
Let there be a list called "NAME" that begins empty.
```

### Appending — Remember

```babel
Remember EXPR as NAME.
```

Reads literally: "remember 3 as primes" appends 3 to the end of the list
`primes`.

```babel
Let there be a list called "primes" that begins empty.
Remember 2 as primes.
Remember 3 as primes.
Remember 5 as primes.
Print primes.    # 2, 3, 5
```

### Reading from a list

This is important: Babel has **no positional indexing**. You cannot ask
for "the 3rd element of this list". The operations you have are:

- `the length of LIST` — the count
- `the sum of LIST` — the numeric total
- `LIST contains EXPR` — membership
- `Print LIST, separated by ...` — printing all of it
- Iterating (indirectly): there is no `for each` over a list. `For every`
  is a counted loop over a numeric range. To walk a list, you typically
  build it in the same shape as the range you'll iterate, or you rephrase
  your algorithm to not need random access.

See the [Cookbook](cookbook.md) for the usual workarounds.

---

## Recipes (functions)

A **recipe** is Babel's word for a function.

### Defining a recipe

```babel
To NAME, do the following:
    BODY

To NAME with ARG, do the following:
    BODY

To NAME with ARG1 and ARG2 and ARG3, do the following:
    BODY
```

The arguments (`ingredients`, in the story voice) are listed with `and`
between them. The body is an indented block.

Rules:

- `NAME` must be a single word. No spaces in recipe names.
- Argument names follow the same rules as ordinary identifiers, **with
  one exception**: you cannot use `a` as an argument name, because `a`
  is a keyword. Use `x`, `n`, `left`, `first`, etc.
- A recipe can call itself by name. Recursion is supported.

### Returning a value

```babel
The answer is EXPR.
Give back EXPR.
```

The two forms are identical. Either one causes the recipe to end
immediately and produce `EXPR` as its result. A recipe can have several
`The answer is` lines along different branches.

```babel
To primality with n, do the following:
    If n is less than 2, the answer is false.
    For every divisor from 2 to n minus 1, do the following:
        If n is divisible by divisor, the answer is false.
    The answer is true.
```

A recipe that never says `The answer is` anything returns `nothing`.

### Calling a recipe

A recipe call can appear in two places:

**As an expression** — inside `Print`, `Set`, `Let`, etc.:

```babel
Print double with 21.
Let there be a number called "g" that equals gcd with 48 and 18.
```

**As a statement** on its own:

```babel
double with 21.
```

When the call has arguments, you separate them with `and`:

```babel
Print gcd with 48 and 18.
Print roman with 1984.
```

### Scope

Each recipe call runs in a fresh scope. Ingredients are bound to their
argument names inside that scope. A recipe can see the names declared at
the top level of the program (lexical scope). It cannot see the caller's
local variables.

An assignment to an outer variable from inside a recipe *does* update
the outer variable — assignment walks up the scope chain to find an
existing binding.

---

## Pronouns

Two small words let you refer to things without naming them again.

### it

`it` refers to the **most recently named** variable.

```babel
Let there be a number called "x" that equals 10.
Print it.           # prints 10
```

Use it sparingly — it's easy to lose track of what "it" means if there
are several names nearby.

### our

`our X` is a synonym for `X`. It reads more naturally in some
sentences and has no other effect.

```babel
Print our score.
```

---

## Command line

### Running a file

```
$ ./babel path/to/program.babel
```

The default path runs the **interpreter** — the tree-walking evaluator.
This is the fastest way to try something, and it's the path with the
most complete feature coverage.

### Compiling to a native binary

```
$ ./babel -c -o myprog path/to/program.babel
$ ./myprog
```

Babel writes a C file, hands it to `cc -O3`, and produces a native
binary. The binary runs at the speed of optimized C.

Note: the native backend does not yet support the `contains` operator.
If you need `contains`, use the interpreter.

### Transcribing to Python

```
$ ./babel -p -o myprog.py path/to/program.babel
$ python3 myprog.py
```

Produces a Python program equivalent to the Babel source. Useful if you
want to read what the program does in a language you already know, or
drop the result into a larger Python project.

Limitations: Python's division always yields a float, so programs that
rely on "integer-looking" output after `divided by` may differ by a
`.0` from the interpreter's output. And, as with the C backend, the
Python transcriber does not handle `contains`.

### The REPL

```
$ ./babel
```

Drops into an interactive shell. Type a statement, press enter, see the
result. Unfinished blocks (lines ending in `:`) prompt with `...` until
you enter an empty line. `Ctrl-D` exits.

### The loose-English front-end

```
$ ./babel -i                        # interactive, freeform English
$ ./babel -i path/to/program.babel  # pre-process a file through it
```

Starts (or runs) the **Interpreter of Tongues**. She accepts freeform
English — phrases like *show me*, *make a*, *loop through*, *whenever*,
*bigger than*, *same as* — and translates them into strict Babel before
handing the result to the tower. See the [Getting started page](getting-started.md#5-the-friendly-front-end).
