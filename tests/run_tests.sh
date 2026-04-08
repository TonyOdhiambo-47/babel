#!/bin/sh
# A small choir of voices that asks the tower the same questions
# every morning, to make sure the tower still knows the answers.
#
# Each test is a Babel program and the words it should speak back.
# If the tower answers differently, the test fails.

set -e

cd "$(dirname "$0")/.."

if [ ! -x ./babel ]; then
    make >/dev/null
fi

passed=0
failed=0
failures=""

check() {
    name="$1"
    program="$2"
    expected="$3"

    actual=$(printf '%s\n' "$program" | ./babel /dev/stdin 2>&1 || true)
    if [ "$actual" = "$expected" ]; then
        passed=$((passed + 1))
        printf "  ok    %s\n" "$name"
    else
        failed=$((failed + 1))
        failures="$failures $name"
        printf "  FAIL  %s\n" "$name"
        printf "        expected: %s\n" "$expected"
        printf "        actual:   %s\n" "$actual"
    fi
}

check_example() {
    name="$1"
    file="$2"
    expected="$3"

    actual=$(./babel "$file" 2>&1 || true)
    if [ "$actual" = "$expected" ]; then
        passed=$((passed + 1))
        printf "  ok    %s\n" "$name"
    else
        failed=$((failed + 1))
        failures="$failures $name"
        printf "  FAIL  %s\n" "$name"
        printf "        expected: %s\n" "$expected"
        printf "        actual:   %s\n" "$actual"
    fi
}

echo "the tower of babel — daily questions"
echo

check "the smallest sentence" \
    'Print "hello".' \
    'hello'

check "a number remembered" \
    'Let there be a number called "x" that equals 7.
Print x.' \
    '7'

check "addition" \
    'Let there be a number called "x" that equals 2.
Let there be a number called "y" that equals 3.
Print x plus y.' \
    '5'

check "set with self-reference" \
    'Let there be a number called "score" that equals 41.
Set score to score plus 1.
Print score.' \
    '42'

check "a small loop" \
    'For every i from 1 to 3, do the following:
    Print i.' \
    '1
2
3'

check "an if that takes the true path" \
    'If 2 plus 2 equals 4, print "yes".' \
    'yes'

check "an if that takes the false path" \
    'If 2 plus 2 equals 5, print "yes".
Otherwise, print "no".' \
    'no'

check "a list and its length" \
    'Let there be a list called "xs" that begins empty.
Remember 1 as xs.
Remember 2 as xs.
Remember 3 as xs.
Print the length of xs.' \
    '3'

check "a list printed with a separator" \
    'Let there be a list called "xs" that begins empty.
Remember 1 as xs.
Remember 2 as xs.
Remember 3 as xs.
Print xs, separated by ", ".' \
    '1, 2, 3'

check "a recipe with one argument" \
    'To double with n, do the following:
    The answer is n times 2.
Print double with 21.' \
    '42'

echo
echo "and now the longer scrolls in examples/"
echo

check_example "examples/hello.babel" examples/hello.babel \
    "Hello, world"

check_example "examples/primes.babel (first 100)" examples/primes.babel \
    "2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97"

check_example "examples/factorial.babel" examples/factorial.babel \
    "1
2
6
24
120
720
5040
40320
362880
3628800"

echo
echo "and now the Scribe's Python transcriptions"
echo

if command -v python3 >/dev/null 2>&1; then
    for e in hello fizzbuzz primes factorial fibonacci poetry; do
        expected=$(./babel "examples/$e.babel" 2>&1 || true)
        actual=$(./babel -p "examples/$e.babel" 2>/dev/null | python3 2>&1 || true)
        if [ "$expected" = "$actual" ]; then
            passed=$((passed + 1))
            printf "  ok    examples/%s.babel (python transcription)\n" "$e"
        else
            failed=$((failed + 1))
            failures="$failures $e.py"
            printf "  FAIL  examples/%s.babel (python transcription)\n" "$e"
            printf "        evaluator said: %s\n" "$expected"
            printf "        python     said: %s\n" "$actual"
        fi
    done
else
    printf "  skip  no python3 on PATH\n"
fi

echo
printf "passed: %d   failed: %d\n" "$passed" "$failed"

if [ "$failed" -gt 0 ]; then
    echo "the tower has cracks:$failures"
    exit 1
fi
echo "the tower stands."

# And finally, a few questions for the Interpreter of Tongues,
# so we know she is still hearing us.
if [ -x tests/run_interpreter_tests.sh ]; then
    echo
    tests/run_interpreter_tests.sh
fi
