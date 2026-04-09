# Babel documentation

Babel is a small language where the source code is structured English. These
pages are the reference for how to write it.

If you've never run Babel before, start with **[Getting started](getting-started.md)**.
If you want to look up a specific rule — how `While` works, how to define a
recipe, what `Remember as` does — go to the **[Language reference](language.md)**.

## Pages

- **[Getting started](getting-started.md)** — building the compiler, running
  your first program, and how the REPL works.
- **[Language reference](language.md)** — the full set of rules: lexing,
  types, declarations, expressions, conditionals, loops, recipes, lists,
  and the command-line interface.
- **[Cookbook](cookbook.md)** — short idiomatic snippets for the things you
  will reach for most often: summing, filtering, recursion, string-building,
  breaking out of a loop, and so on.
- **[Gotchas and limits](gotchas.md)** — the rough edges you will meet if you
  write anything non-trivial. Read this *before* you spend an hour debugging
  something the language simply does not support.
- **[The living dashboard](../babel-dashboard/README.md)** — a browser window
  onto the compiler, with voice input, live translation, and streaming output.

## A one-screen tour

```babel
# A comment line begins with a hash mark.

Let there be a number called "x" that equals 7.
Let there be a word called "greeting" that equals "hello, ".

Say greeting plus "world".
Print x times x.

For every i from 1 to 5, do the following:
    If i is divisible by 2, say "even".
    Otherwise, say "odd".

To double with n, do the following:
    The answer is n times 2.

Print double with 21.
```

Run it:

```
$ ./babel your-program.babel
```

That's most of the language on one screen. The rest — lists, `While`,
recursion, `Stop`, the loose-English front-end — builds on these same
pieces.
