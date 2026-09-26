#include <algorithm>
#include <format>
#include <numeric>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

constexpr int kCells = 18;

// `count` distinct values from 1..99, in random order.
std::vector<int> distinct_values(Viz& viz, int count) {
  std::vector<int> values(99);
  std::iota(values.begin(), values.end(), 1);
  std::ranges::shuffle(values, viz.rng());
  values.resize(static_cast<std::size_t>(count));
  return values;
}

std::string draw_array(const std::vector<int>& values, const auto& color_of) {
  std::string body;
  for (int i = 0; i < kCells; ++i) body += paint(color_of(i), std::format("{:>4}", values[i]));
  body += '\n';
  for (int i = 0; i < kCells; ++i) body += paint(ansi::gray, std::format("{:>4}", i));
  return body;
}

bool linear_search(Viz& viz) {
  const std::vector<int> values = distinct_values(viz, kCells);
  const int target = values[static_cast<std::size_t>(viz.random(0, kCells - 1))];
  int found = -1;
  for (int i = 0; i < kCells && found < 0; ++i) {
    if (values[i] == target) found = i;
    const auto color_of = [&](int k) {
      if (k == found) return ansi::green;
      return k == i ? ansi::yellow : k < i ? ansi::gray : ansi::white;
    };
    viz.show(draw_array(values, color_of), std::format("looking for {}: index {}", target, i));
  }
  return found >= 0 && values[found] == target;
}

bool binary_search(Viz& viz) {
  std::vector<int> values = distinct_values(viz, kCells);
  std::ranges::sort(values);
  const int target = values[static_cast<std::size_t>(viz.random(0, kCells - 1))];
  int lo = 0;
  int hi = kCells - 1;
  int found = -1;
  while (lo <= hi && found < 0) {
    const int mid = lo + (hi - lo) / 2;
    if (values[mid] == target) found = mid;
    const auto color_of = [&](int k) {
      if (k == found) return ansi::green;
      return k == mid ? ansi::yellow : k >= lo && k <= hi ? ansi::white : ansi::gray;
    };
    viz.show(draw_array(values, color_of),
             std::format("looking for {}: lo={} mid={} hi={}", target, lo, mid, hi));
    if (values[mid] < target) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  return found >= 0 && values[found] == target;
}

// Knuth-Morris-Pratt: on a mismatch, the failure table says how much of the match to keep.
bool kmp_search(Viz& viz) {
  const std::string pattern = "abaab";
  const int m = static_cast<int>(pattern.size());
  std::string text(64, 'a');
  for (char& c : text) c = viz.random(0, 1) ? 'a' : 'b';
  for (int copies = 0; copies < 2; ++copies) {
    text.replace(static_cast<std::size_t>(viz.random(0, 64 - m)), pattern.size(), pattern);
  }
  const int n = static_cast<int>(text.size());

  // failure[i]: length of the longest proper prefix of pattern[0..i] that is also its suffix.
  std::vector<int> failure(pattern.size(), 0);
  for (int i = 1, k = 0; i < m; ++i) {
    while (k > 0 && pattern[i] != pattern[k]) k = failure[k - 1];
    if (pattern[i] == pattern[k]) ++k;
    failure[i] = k;
  }

  std::vector<int> matches;
  const auto draw = [&](int i, int start, std::string_view status) {
    std::string body;
    for (int k = 0; k < n; ++k) {
      const bool in_match = std::ranges::any_of(matches, [&](int s) { return k >= s && k < s + m; });
      const std::string_view c = k == i ? ansi::yellow : in_match ? ansi::green : ansi::white;
      body += paint(c, std::string(1, text[k]));
    }
    body += '\n' + std::string(static_cast<std::size_t>(start), ' ');
    for (int k = 0; k < m; ++k) {
      body += paint(start + k < i ? ansi::cyan : start + k == i ? ansi::yellow : ansi::gray,
                    std::string(1, pattern[k]));
    }
    body += "\n\nfailure table: ";
    for (const int f : failure) body += std::format("{} ", f);
    viz.show(body, std::format("{}   matches: {}", status, matches.size()));
  };

  for (int i = 0, j = 0; i < n; ++i) {  // j: pattern characters matched so far
    while (j > 0 && text[i] != pattern[j]) {
      draw(i, i - j, std::format("mismatch at {}: keep {} chars", i, failure[j - 1]));
      j = failure[j - 1];
    }
    if (text[i] == pattern[j]) ++j;
    if (j == m) {
      matches.push_back(i - m + 1);
      draw(i, i - j + 1, std::format("match at {}", i - m + 1));
      j = failure[j - 1];
    } else {
      draw(i, i - j + 1, std::format("compare text[{}]", i));
    }
  }

  std::vector<int> expected;
  for (auto p = text.find(pattern); p != std::string::npos; p = text.find(pattern, p + 1)) {
    expected.push_back(static_cast<int>(p));
  }
  return matches == expected;
}

}  // namespace

std::vector<Algorithm> searching_algorithms() {
  return {
      {"linear-search", "searching", "Check every element in turn. O(n)", linear_search, 150},
      {"binary-search", "searching", "Halve a sorted range each step. O(log n)", binary_search, 700},
      {"kmp-search", "searching", "Knuth-Morris-Pratt string matching with a failure table. O(n+m)",
       kmp_search, 80},
  };
}

}  // namespace algoviz
