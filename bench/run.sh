#!/bin/sh
# A small footrace between three runners:
#   - Babel tree walker            (./babel bench/primes.babel)
#   - Babel C scribe (native -O3)  (./babel -c, then ./out)
#   - Hand-written Python          (python3 bench/primes.py)
#
# All three compute the number of primes below 5000. They should
# all say the same number. Only the speeds should differ.

set -e
cd "$(dirname "$0")/.."

if [ ! -x ./babel ]; then make >/dev/null; fi

time_it() {
    label="$1"; shift
    start=$(python3 -c 'import time;print(time.time())')
    out=$("$@" 2>&1)
    end=$(python3 -c 'import time;print(time.time())')
    elapsed=$(python3 -c "print(f'{$end - $start:.3f}')")
    printf "  %-32s  %8ss   -> %s\n" "$label" "$elapsed" "$out"
}

echo "bench: count primes below 5000"
echo

./babel -c -o /tmp/bench_primes bench/primes.babel >/dev/null

time_it "babel (tree walker)"          ./babel bench/primes.babel
time_it "babel (C scribe, -O3)"        /tmp/bench_primes
time_it "hand-written python"          python3 bench/primes.py

echo
echo "all three runners should have said the same number."
rm -f /tmp/bench_primes
