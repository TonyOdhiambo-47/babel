# Gotchas and limits

Babel is deliberately small. If something isn't here, it probably isn't
supported. This page is the list of rough edges you will hit if you try
to write anything real, so you don't spend an hour debugging something
the language simply does not do.

## Things Babel does not have

### No list indexing

You cannot ask for "the Nth element of this list". The only things you
can do with a list are:

- append to it with `Remember ... as ...`
- read its length (`the length of xs`)
- read its total (`the sum of xs`)
- ask whether it contains a value (`xs contains 7`)
- print the whole thing

If your algorithm needs positional access to a list, you will have to
rephrase it. The [Cookbook](cookbook.md#finding-a-maximum-within-a-known-range)
has the usual workarounds.

### No `for each` over a list

`For every i from A to B` is a **counted loop over a numeric range**. It
does not iterate over the elements of a list. If you want to walk a
list, you typically build your program so the interesting data lives in
a range of integers, or you check membership with `contains`.

### No `and` / `or` in conditions

A condition is a **single comparison**. You cannot write
`If x is greater than 0 and x is less than 10, ...`. Compound conditions
are expressed by nesting `If` statements or chaining `Otherwise, if ...`.

### No `>=` or `<=`

The comparators are `is greater than`, `is less than`, `equals`, `is`,
`is not`, `is divisible by`, `is not divisible by`, and `contains`.
That's the whole set. To express `x >= 10`, write `x is greater than 9`
if you are working with integers, or flip the condition.

### No parenthesized expressions

There are no parentheses. Operator precedence is the usual "multiply
before add", left-associative. If you need to force an unusual grouping,
use an intermediate variable:

```babel
# Want: (x minus 1) divided by 2
Let there be a number called "shifted" that equals x minus 1.
Let there be a number called "half" that equals shifted divided by 2.
```

### No negative number literals

`-5` is not a token. Write `0 minus 5`.

### No inline comments

A comment is a whole line whose first non-whitespace character is `#`.
You cannot put `# ...` at the end of a statement. This will break:

```babel
Print "hi".   # this is a syntax error
```

Put the comment on its own line above instead.

### No string escape sequences

A string literal is exactly the characters between the two `"`. There's
no `\n`, `\t`, or `\"`. If you want a tab as a separator, use literal
spaces. If you want a newline inside a single printed line — you can't,
use two `Say` statements.

### Strings are short

Strings live in a fixed 80-character buffer. Longer strings get
truncated. When you `plus`-concatenate words, the result is also
bounded by that buffer. For anything longer, you'll have to use a
different backend or restructure the output as many small prints.

### No conversion between words and numbers

`plus` concatenates two words, and it adds two numbers, but it does
not let you mix. There's no built-in `to-string` for a number. In
practice: build words from words, and print numbers with `Print`.

### Recipe names are one word

`To sum_of_divisors with n, ...` is fine. `To sum of divisors with n, ...`
is not — the parser will only take a single word as the recipe name.
Use underscores.

### `a` is a keyword, not a name

You cannot name an argument `a`, because `a` is a keyword. `gcd with a
and b` will fail to parse. Use different letters: `gcd with x and y`.

### `If` only takes a single statement per branch

```babel
If x is greater than 0, STATEMENT.
Otherwise, STATEMENT.
```

There is no `{ ... }` or indented block form for the body of an `If`.
If you need multiple actions, move them into a recipe and call the
recipe from the branch:

```babel
To handle_positive, do the following:
    Say "positive".
    Set counter to counter plus 1.

If x is greater than 0, handle_positive.
```

### The Interpreter, the C backend, and the Python transcriber are not equal

The default interpreter has the most features. The other two backends
lag behind:

- **Native C backend** (`-c`): does not support `contains`.
- **Python transcription** (`-p`): does not support `contains`; also,
  Python's division always yields a float, so programs that use
  `divided by` on integer values will print `5.0` where the interpreter
  would print `5`.

If you want your program to be portable across all three, avoid
`contains` and avoid printing the result of `divided by` directly.

### Deep recursion can blow the C stack

The evaluator uses the host C stack for recursive calls. Ackermann
`A(3, 5)` and friends will segfault. Keep recursion to depths the C
stack can handle — a few thousand frames is typically fine.

## Things that look like bugs but aren't

### "I said `Print x plus 1.` and got a weird result"

If `x` is a **word** (string), `plus 1` tries to concatenate a number
onto it, and the number side becomes an empty string. The result is
just the word unchanged. Check your variable's type.

### "`For every i from 10 to 1` doesn't do anything"

`For every` is an **inclusive, ascending** counted loop. If the start is
greater than the end, the loop runs zero times. To count downward, use
a `While` loop with `Set n to n minus 1`.

### "My `If` inside a loop printed nothing, even though the condition should have been true"

Double-check the types of the things you're comparing. `equals` compares
by value, but mixing types (a word against a number, for instance)
won't give you what you want. Also check for trailing whitespace in
string literals.

### "My recipe doesn't see a variable I declared right before calling it"

Recipes have **lexical scope**. They see the names that existed at the
top level when the recipe was *defined*, not the names that exist at
the call site. Declare shared state at the top of the file, before the
recipe definition.

---

If you hit something that isn't covered here and it feels wrong, try
the program in the interpreter (`./babel your-file.babel`). The error
messages are written in the same voice as the rest of the system and
will usually point at the specific line.
