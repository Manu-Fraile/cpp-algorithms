#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

constexpr int kBlank = -1;  // table cell not computed yet

using Table = std::vector<std::vector<int>>;

// Renders a DP table with headers; `color_of(row, col)` picks each cell's color.
std::string draw_table(const std::vector<std::string>& row_labels,
                       const std::vector<std::string>& col_labels, const Table& table,
                       const auto& color_of) {
  std::size_t label_width = 0;
  for (const auto& label : row_labels) label_width = std::max(label_width, label.size());
  std::string body(label_width, ' ');
  for (const auto& label : col_labels) body += std::format("{:>4}", label);
  for (std::size_t r = 0; r < table.size(); ++r) {
    body += '\n' + paint(ansi::gray, std::format("{:>{}}", row_labels[r], label_width));
    for (std::size_t c = 0; c < table[r].size(); ++c) {
      const int v = table[r][c];
      body += paint(color_of(static_cast<int>(r), static_cast<int>(c)),
                    v == kBlank ? "   ·" : std::format("{:>4}", v));
    }
  }
  return body;
}

std::vector<std::string> letters(std::string_view word) {
  std::vector<std::string> out{"ε"};
  for (const char c : word) out.emplace_back(1, c);
  return out;
}

// Top-down Fibonacci with memoization: each value is computed once, then looked up.
bool fibonacci(Viz& viz) {
  constexpr int n = 15;
  std::vector<long long> memo(n + 1, -1);
  std::vector<int> calls;  // arguments of the active calls, outermost first

  const auto draw = [&](std::string_view status) {
    std::string body = "  n ";
    for (int k = 0; k <= n; ++k) body += paint(ansi::gray, std::format("{:>4}", k));
    body += "\nF(n)";
    for (int k = 0; k <= n; ++k) {
      const std::string_view color = !calls.empty() && calls.back() == k ? ansi::yellow : ansi::green;
      body += memo[k] < 0 ? std::string("   ·") : paint(color, std::format("{:>4}", memo[k]));
    }
    body += "\n\ncall stack: ";
    for (const int k : calls) body += std::format("fib({}) ", k);
    viz.show(body, status);
  };

  const auto fib = [&](const auto& self, int k) -> long long {
    calls.push_back(k);
    if (memo[k] >= 0) {
      draw(std::format("fib({}) is memoized: {}", k, memo[k]));
    } else {
      draw(std::format("fib({}) not known yet", k));
      memo[k] = k < 2 ? k : self(self, k - 1) + self(self, k - 2);
      draw(std::format("fib({}) = {}", k, memo[k]));
    }
    calls.pop_back();
    return memo[k];
  };
  const long long result = fib(fib, n);

  long long a = 0;
  long long b = 1;
  for (int i = 0; i < n; ++i) a = std::exchange(b, a + b);
  return result == a;
}

bool is_subsequence(std::string_view sub, std::string_view of) {
  std::size_t i = 0;
  for (const char c : of) {
    if (i < sub.size() && sub[i] == c) ++i;
  }
  return i == sub.size();
}

// Longest common subsequence of two random DNA strands.
bool longest_common_subsequence(Viz& viz) {
  std::string a(10, ' ');
  std::string b(9, ' ');
  for (char& c : a) c = "ACGT"[viz.random(0, 3)];
  for (char& c : b) c = "ACGT"[viz.random(0, 3)];
  const std::size_t m = a.size();
  const std::size_t n = b.size();
  Table t(m + 1, std::vector<int>(n + 1, kBlank));
  for (auto& row : t) row[0] = 0;
  std::ranges::fill(t[0], 0);

  std::vector<std::pair<std::size_t, std::size_t>> path;
  const auto on_path = [&](int r, int c) {
    return std::ranges::find(path, std::pair{static_cast<std::size_t>(r), static_cast<std::size_t>(c)}) != path.end();
  };
  for (std::size_t i = 1; i <= m; ++i) {
    for (std::size_t j = 1; j <= n; ++j) {
      t[i][j] = a[i - 1] == b[j - 1] ? t[i - 1][j - 1] + 1 : std::max(t[i - 1][j], t[i][j - 1]);
      const auto color_of = [&](int r, int c) {
        const auto ur = static_cast<std::size_t>(r);
        const auto uc = static_cast<std::size_t>(c);
        if (ur == i && uc == j) return ansi::yellow;
        return ur + 1 >= i && ur <= i && uc + 1 >= j && uc <= j ? ansi::cyan : ansi::white;
      };
      viz.show(draw_table(letters(a), letters(b), t, color_of),
               std::format("{} vs {}: {}", a[i - 1], b[j - 1], a[i - 1] == b[j - 1] ? "match, diagonal + 1" : "max of top and left"));
    }
  }

  std::string lcs;
  for (std::size_t i = m, j = n; i > 0 && j > 0;) {
    path.emplace_back(i, j);
    if (a[i - 1] == b[j - 1]) {
      lcs.insert(lcs.begin(), a[i - 1]);
      --i;
      --j;
    } else if (t[i - 1][j] >= t[i][j - 1]) {
      --i;
    } else {
      --j;
    }
    const auto color_of = [&](int r, int c) { return on_path(r, c) ? ansi::green : ansi::gray; };
    viz.show(draw_table(letters(a), letters(b), t, color_of), std::format("backtrack: \"{}\"", lcs));
  }
  return static_cast<int>(lcs.size()) == t[m][n] && is_subsequence(lcs, a) && is_subsequence(lcs, b);
}

// Levenshtein distance: fewest insertions, deletions and substitutions turning one word into another.
bool edit_distance(Viz& viz) {
  struct Case {
    std::string_view from;
    std::string_view to;
    int expected;
  };
  constexpr std::array<Case, 4> kCases{{{"kitten", "sitting", 3},
                                        {"intention", "execution", 5},
                                        {"sunday", "saturday", 3},
                                        {"flaw", "lawn", 2}}};
  const Case& test = kCases[static_cast<std::size_t>(viz.random(0, 3))];
  const std::string_view a = test.from;
  const std::string_view b = test.to;
  Table t(a.size() + 1, std::vector<int>(b.size() + 1, kBlank));
  for (std::size_t i = 0; i <= a.size(); ++i) t[i][0] = static_cast<int>(i);
  for (std::size_t j = 0; j <= b.size(); ++j) t[0][j] = static_cast<int>(j);
  for (std::size_t i = 1; i <= a.size(); ++i) {
    for (std::size_t j = 1; j <= b.size(); ++j) {
      const int substitute = t[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
      t[i][j] = std::min({substitute, t[i - 1][j] + 1, t[i][j - 1] + 1});
      const auto color_of = [&](int r, int c) {
        const auto ur = static_cast<std::size_t>(r);
        const auto uc = static_cast<std::size_t>(c);
        if (ur == i && uc == j) return ansi::yellow;
        return ur + 1 >= i && ur <= i && uc + 1 >= j && uc <= j ? ansi::cyan : ansi::white;
      };
      viz.show(draw_table(letters(a), letters(b), t, color_of),
               std::format("{} → {}: min(substitute, delete, insert) = {}", a, b, t[i][j]));
    }
  }
  return t[a.size()][b.size()] == test.expected;
}

// 0/1 knapsack: best value for each (items considered, capacity) pair.
bool knapsack(Viz& viz) {
  constexpr int kItems = 5;
  constexpr int kCapacity = 12;
  std::vector<int> weight(kItems);
  std::vector<int> value(kItems);
  std::vector<std::string> row_labels{"none"};
  for (std::size_t i = 0; i < kItems; ++i) {
    weight[i] = viz.random(1, 6);
    value[i] = viz.random(1, 20);
    row_labels.push_back(std::format("w{} v{}", weight[i], value[i]));
  }
  std::vector<std::string> col_labels;
  for (int c = 0; c <= kCapacity; ++c) col_labels.push_back(std::to_string(c));

  Table t(kItems + 1, std::vector<int>(kCapacity + 1, kBlank));
  std::ranges::fill(t[0], 0);
  for (std::size_t i = 1; i <= kItems; ++i) {
    for (std::size_t c = 0; c <= kCapacity; ++c) {
      const auto w = static_cast<std::size_t>(weight[i - 1]);
      t[i][c] = t[i - 1][c];
      if (w <= c) t[i][c] = std::max(t[i][c], t[i - 1][c - w] + value[i - 1]);
      const auto color_of = [&](int r, int col) {
        const auto ur = static_cast<std::size_t>(r);
        const auto uc = static_cast<std::size_t>(col);
        if (ur == i && uc == c) return ansi::yellow;
        return ur + 1 == i && (uc == c || (w <= c && uc == c - w)) ? ansi::cyan : ansi::white;
      };
      viz.show(draw_table(row_labels, col_labels, t, color_of),
               std::format("item {} at capacity {}: skip it or take it", i, c));
    }
  }

  int best = 0;  // brute force over all 2^n subsets
  for (unsigned mask = 0; mask < (1U << kItems); ++mask) {
    int w = 0;
    int v = 0;
    for (std::size_t i = 0; i < kItems; ++i) {
      if ((mask >> i) & 1U) {
        w += weight[i];
        v += value[i];
      }
    }
    if (w <= kCapacity) best = std::max(best, v);
  }
  return t[kItems][kCapacity] == best;
}

}  // namespace

std::vector<Algorithm> dynamic_programming_algorithms() {
  return {
      {"fibonacci", "dynamic-programming", "Memoized recursion: each F(n) computed once", fibonacci,
       250},
      {"lcs", "dynamic-programming", "Longest common subsequence of two DNA strands",
       longest_common_subsequence, 60},
      {"edit-distance", "dynamic-programming", "Levenshtein distance between two words",
       edit_distance, 80},
      {"knapsack", "dynamic-programming", "0/1 knapsack: best value within a weight limit", knapsack,
       40},
  };
}

}  // namespace algoviz
