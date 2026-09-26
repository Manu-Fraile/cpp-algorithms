#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

bool is_prime_by_trial_division(int n) {
  if (n < 2) return false;
  for (int d = 2; d * d <= n; ++d) {
    if (n % d == 0) return false;
  }
  return true;
}

// Sieve of Eratosthenes: cross out every multiple of each prime, starting at its square.
bool sieve_of_eratosthenes(Viz& viz) {
  constexpr int kLimit = 200;
  constexpr int kPerRow = 20;
  std::vector<bool> composite(kLimit + 1, false);
  const auto draw = [&](int prime, int crossing) {
    std::string body;
    for (int k = 1; k <= kLimit; ++k) {
      std::string_view color = ansi::white;  // not decided yet
      if (k == crossing) {
        color = ansi::red;
      } else if (k == prime) {
        color = ansi::yellow;
      } else if (k < 2 || composite[k]) {
        color = ansi::gray;
      } else if (k < prime || prime * prime > kLimit) {
        color = ansi::green;  // no smaller prime divides it
      }
      body += paint(color, std::format("{:>4}", k));
      if (k % kPerRow == 0) body += '\n';
    }
    viz.show(body, crossing > 0 ? std::format("{} is a multiple of {}", crossing, prime)
                                : std::format("{} is prime", prime));
  };
  for (int p = 2; p * p <= kLimit; ++p) {
    if (composite[p]) continue;
    draw(p, 0);
    for (int m = p * p; m <= kLimit; m += p) {
      composite[m] = true;
      draw(p, m);
    }
  }
  draw(kLimit, 0);
  for (int k = 2; k <= kLimit; ++k) {
    if (composite[k] == is_prime_by_trial_division(k)) return false;
  }
  return true;
}

// Collatz conjecture: halve even numbers, map odd n to 3n + 1; every start seems to reach 1.
bool collatz(Viz& viz) {
  constexpr int kColumns = 70;
  constexpr int kRows = 16;
  std::vector<long long> sequence{viz.random(20, 999)};
  while (sequence.back() != 1 && sequence.size() < 10'000) {
    const long long n = sequence.back();
    sequence.push_back(n % 2 == 0 ? n / 2 : 3 * n + 1);

    // Chart the latest kColumns steps, scaled to the peak so far.
    const long long peak = std::ranges::max(sequence);
    const std::size_t first = sequence.size() > kColumns ? sequence.size() - kColumns : 0;
    std::string body;
    for (int row = kRows; row >= 1; --row) {
      for (std::size_t i = first; i < sequence.size(); ++i) {
        const bool filled = sequence[i] * kRows > (row - 1) * peak;
        const bool current = i + 1 == sequence.size();
        body += filled ? paint(current ? ansi::yellow : ansi::cyan, "█") : " ";
      }
      body += '\n';
    }
    viz.show(body, std::format("start {}   step {}   value {}   peak {}", sequence.front(),
                               sequence.size() - 1, sequence.back(), peak));
  }
  for (std::size_t i = 1; i < sequence.size(); ++i) {
    const long long n = sequence[i - 1];
    if (sequence[i] != (n % 2 == 0 ? n / 2 : 3 * n + 1)) return false;
  }
  return sequence.back() == 1;
}

// Goldbach's conjecture: every even number greater than 2 is the sum of two primes.
bool goldbach(Viz& viz) {
  constexpr int kLimit = 100;
  std::vector<int> primes;
  for (int k = 2; k <= kLimit; ++k) {
    if (is_prime_by_trial_division(k)) primes.push_back(k);
  }
  std::vector<std::string> found;
  bool ok = true;
  for (int n = 4; n <= kLimit; n += 2) {
    for (const int p : primes) {
      const int q = n - p;
      const bool hit = is_prime_by_trial_division(q);
      std::string body;
      for (const int k : primes) {
        const std::string_view color = k == p || (hit && k == q) ? (hit ? ansi::green : ansi::yellow)
                                                               : ansi::gray;
        body += paint(color, std::format("{:>3}", k));
      }
      body += "\n\n";
      const std::size_t shown = std::min<std::size_t>(found.size(), 8);
      for (std::size_t i = found.size() - shown; i < found.size(); ++i) body += found[i] + '\n';
      viz.show(body, std::format("{} - {} = {} {}", n, p, q, hit ? "is prime ✓" : "is not prime"));
      if (hit) {
        found.push_back(std::format("{} = {} + {}", n, p, q));
        ok = ok && is_prime_by_trial_division(p) && p + q == n;
        break;
      }
    }
  }
  return ok && found.size() == (kLimit - 4) / 2 + 1;
}

}  // namespace

std::vector<Algorithm> mathematics_algorithms() {
  return {
      {"sieve", "mathematics", "Sieve of Eratosthenes: cross out multiples of each prime",
       sieve_of_eratosthenes, 40},
      {"collatz", "mathematics", "Collatz sequence: n/2 if even, 3n+1 if odd, until 1", collatz,
       60},
      {"goldbach", "mathematics", "Split even numbers into two primes", goldbach, 60},
  };
}

}  // namespace algoviz
