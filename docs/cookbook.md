# Cookbook

Short, working snippets for the things you'll reach for most often.
Every snippet here has been run through the interpreter and produces
the output shown.

## Summing a list

```babel
Let there be a list called "xs" that begins empty.
Remember 10 as xs.
Remember 20 as xs.
Remember 30 as xs.
Print the sum of xs.   # 60
```

## Averaging a list

```babel
Let there be a number called "avg" that equals the sum of xs divided by the length of xs.
Print avg.
```

## Filtering into a new list

Because Babel has no `filter` built-in and no way to iterate a list by
position, the idiom is to build the filtered list by running a counted
loop over the range you care about:

```babel
Let there be a list called "evens" that begins empty.
For every n from 1 to 20, do the following:
    If n is divisible by 2, remember n as evens.
Print evens, separated by ", ".   # 2, 4, 6, 8, 10, 12, 14, 16, 18, 20
```

## Finding a maximum (within a known range)

Babel has no positional access to list elements, so the usual "walk the
list and keep a running max" is awkward. When your data is built from a
range of integers, the cleanest trick is to run the range in order and
let later values overwrite earlier ones:

```babel
Let there be a list called "numbers" that begins empty.
Remember 23 as numbers.
Remember 7  as numbers.
Remember 99 as numbers.
Remember 42 as numbers.

Let there be a number called "biggest" that equals 0.
For every candidate from 1 to 100, do the following:
    If numbers contains candidate, set "biggest" to candidate.
Print biggest.   # 99
```

## Recursion (factorial)

```babel
To factorial with n, do the following:
    If n is less than 2, the answer is 1.
    Let there be a number called "smaller" that equals factorial with n minus 1.
    The answer is n times smaller.

Print factorial with 6.   # 720
```

## Recursion with multiple arguments (GCD)

```babel
To gcd with x and y, do the following:
    If y equals 0, the answer is x.
    The answer is gcd with y and x modulo y.

Print gcd with 48 and 18.   # 6
```

## Building a string row by row

```babel
For every row from 1 to 5, do the following:
    Let there be a word called "line" that equals "".
    For every col from 1 to row, do the following:
        Set "line" to line plus "*".
    Say line.
```

Output:

```
*
**
***
****
*****
```

## Counting with a while loop

```babel
Let there be a number called "n" that equals 10.
While n is greater than 0, do the following:
    Print n.
    Set n to n minus 1.
```

## Breaking out of a loop

```babel
For every i from 1 to 100, do the following:
    If i equals 7, stop.
    Print i.
```

Output: `1 2 3 4 5 6` (each on its own line).

## A compound condition by nesting

There's no `and` / `or` in conditions, so multi-part checks nest or
chain through `Otherwise`:

```babel
# Leap year, Gregorian rule:
If year is divisible by 400, say "leap".
Otherwise, if year is not divisible by 100, if year is divisible by 4, say "leap".
```

## A running counter updated by a recipe

An assignment inside a recipe walks up the scope chain, so a recipe can
mutate a top-level variable:

```babel
Let there be a number called "steps" that equals 0.

To take_a_step, do the following:
    Set steps to steps plus 1.

take_a_step.
take_a_step.
take_a_step.
Print steps.   # 3
```

## Integer digit extraction

To pull digits off the right side of a number, use `modulo 10` for the
last digit and a subtract-then-divide pattern for the rest. Remember
there are no parentheses, so use an intermediate variable to force the
order:

```babel
Let there be a number called "n" that equals 345.
While n is greater than 0, do the following:
    Let there be a number called "digit" that equals n modulo 10.
    Print digit.
    Let there be a number called "without_digit" that equals n minus digit.
    Set "n" to without_digit divided by 10.
```

Output: `5 4 3` (right-to-left, each on its own line).

## Pretty-printing a list with a custom separator

```babel
Let there be a list called "primes" that begins empty.
Remember 2 as primes.
Remember 3 as primes.
Remember 5 as primes.
Remember 7 as primes.

Print primes, separated by " • ".   # 2 • 3 • 5 • 7
```

## A recipe that returns a truth

```babel
To is_even with n, do the following:
    If n is divisible by 2, the answer is true.
    The answer is false.

If is_even with 10 equals true, say "ten is even".
```

---

If you run into a pattern that isn't here and you suspect Babel might
not support it, the [Gotchas page](gotchas.md) is where the known
missing features live.
