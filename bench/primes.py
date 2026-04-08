primes = []
for candidate in range(2, 5001):
    looks_prime = True
    for tester in range(2, candidate):
        if candidate % tester == 0:
            looks_prime = False
    if looks_prime:
        primes.append(candidate)
print(len(primes))
