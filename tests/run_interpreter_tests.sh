#!/bin/sh
# A small set of questions for the Interpreter of Tongues. Each
# test hands her a freeform English sentence, answers "yes" to
# every confirmation she asks for, and checks that the tower ends
# up speaking the right words.

set -e
cd "$(dirname "$0")/.."

if [ ! -x ./babel ]; then make >/dev/null; fi

passed=0
failed=0
failures=""

# $1: name   $2: freeform input (single line)   $3: expected final output line
check_interp() {
    name="$1"
    program="$2"
    expected="$3"

    # feed the program, answer every question with "yes", then leave.
    # The Interpreter's chrome (banner, confirmations, marks) is
    # either indented or starts with "you>" — real program output
    # comes back left-justified. We keep only the left-justified
    # non-empty lines, skipping the closing "The tower stands."
    actual=$(printf '%s\nyes\nyes\nyes\nyes\nyes\nyes\nGoodbye.\n' "$program" \
        | ./babel -i 2>&1 \
        | sed 's/\x1b\[[0-9;]*m//g' \
        | awk '/^[^ ]/ && !/^you>/ && !/^The tower stands/ {print}' \
        | awk 'NF' \
        | tail -n 1)
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

echo "the interpreter of tongues — short questions"
echo

check_interp "show me a string literal" \
    'show me "hello"' \
    'hello'

check_interp "make a number and show it" \
    'make a number called "x" that equals 7. show me x.' \
    '7'

check_interp "a loose addition" \
    'make a number called "left" that equals 2. make a number called "right" that equals 3. show me left plus right.' \
    '5'

echo
printf "passed: %d   failed: %d\n" "$passed" "$failed"
if [ "$failed" -gt 0 ]; then
    echo "the interpreter is still learning:$failures"
    exit 1
fi
echo "the interpreter hears us."
