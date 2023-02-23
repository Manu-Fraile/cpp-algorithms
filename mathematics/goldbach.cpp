#include <iostream>
#include <vector>
#include <cmath>
#include <bits/stdc++.h>


std::bitset<500000001> Primes;
std::vector<long> erathostenes(long n) {
    std::vector<long> prime_numbers;
    Primes[0] = true;
    for (int i = 3; i*i <= n; i += 2) {
        if (Primes[i / 2] == 0) {
            for (int j = 3 * i; j <= n; j += 2 * i)
                Primes[j / 2] = true;
        }
    }

    for (int i = 1; i <= n; i++) {
        if (i % 2 == 1 && Primes[i / 2] == 0)
            prime_numbers.push_back(i);;
    }

    return prime_numbers;
}


std::vector<long> goldbach(long input){
    std::vector<long> prime_numbers = erathostenes(input);
    std::vector<long> solution;
    long prime_numbers_size = (long)prime_numbers.size();

    for (long i : prime_numbers) {
        for (long j=prime_numbers_size - 1; j >= prime_numbers_size/2; --j) {
            if (i + prime_numbers[j] == input) {
                solution.push_back(i);
                solution.push_back(prime_numbers[j]);
                return solution;
            }
        }
    }
    solution = {0, 0};
    return solution;
}

int main() {
    long input = 579387418;
    std::vector<long> output = goldbach(input);

    std::cout << "The Goldbach sum of " <<  input << " is: " << output.front() << " + " << output.back() << std::endl;
}
