#include <algorithm>
#include <format>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

constexpr int kBarCount = 36;
constexpr int kChartHeight = 16;

// A shuffled array that redraws itself on every read and write, counting both.
class Bars {
 public:
  explicit Bars(Viz& viz) : viz_(viz), values_(kBarCount) {
    std::iota(values_.begin(), values_.end(), 1);
    std::ranges::shuffle(values_, viz.rng());
  }

  [[nodiscard]] int size() const { return kBarCount; }

  int get(int i) {
    ++reads_;
    draw(i, i);
    return at(i);
  }

  void set(int i, int value) {
    ++writes_;
    at(i) = value;
    draw(i, i, ansi::red);
  }

  bool less(int i, int j) {
    reads_ += 2;
    draw(i, j);
    return at(i) < at(j);
  }

  void swap(int i, int j) {
    reads_ += 2;
    writes_ += 2;
    std::swap(at(i), at(j));
    draw(i, j, ansi::red);
  }

  // Sweeps the bars green and reports whether they really are sorted.
  bool finish() {
    for (sorted_ = 1; sorted_ <= size(); ++sorted_) draw(-1, -1);
    return std::ranges::is_sorted(values_);
  }

 private:
  int& at(int i) { return values_[static_cast<std::size_t>(i)]; }

  void draw(int i, int j, std::string_view color = ansi::yellow) {
    std::string body;
    for (int row = kChartHeight; row >= 1; --row) {
      for (int k = 0; k < size(); ++k) {
        // A bar of value v is ceil(v * height / count) rows tall.
        if (at(k) * kChartHeight <= (row - 1) * kBarCount) {
          body += "  ";
          continue;
        }
        const std::string_view c = k == i || k == j ? color : k < sorted_ ? ansi::green : ansi::cyan;
        body += paint(c, "█ ");
      }
      body += '\n';
    }
    viz_.show(body, std::format("reads: {}   writes: {}", reads_, writes_));
  }

  Viz& viz_;
  std::vector<int> values_;
  int reads_ = 0;
  int writes_ = 0;
  int sorted_ = 0;  // bars [0, sorted_) are drawn green
};

bool bubble_sort(Viz& viz) {
  Bars a(viz);
  for (int end = a.size() - 1; end > 0; --end) {
    bool swapped = false;
    for (int i = 0; i < end; ++i) {
      if (a.less(i + 1, i)) {
        a.swap(i, i + 1);
        swapped = true;
      }
    }
    if (!swapped) break;
  }
  return a.finish();
}

bool selection_sort(Viz& viz) {
  Bars a(viz);
  for (int i = 0; i < a.size() - 1; ++i) {
    int min = i;
    for (int j = i + 1; j < a.size(); ++j) {
      if (a.less(j, min)) min = j;
    }
    if (min != i) a.swap(i, min);
  }
  return a.finish();
}

// Shift larger elements one slot right, then drop the key into the gap.
bool insertion_sort(Viz& viz) {
  Bars a(viz);
  for (int i = 1; i < a.size(); ++i) {
    const int key = a.get(i);
    int j = i - 1;
    for (; j >= 0; --j) {
      const int value = a.get(j);
      if (value <= key) break;
      a.set(j + 1, value);
    }
    a.set(j + 1, key);
  }
  return a.finish();
}

// Insertion sort over shrinking gaps, so elements travel far early on.
bool shell_sort(Viz& viz) {
  Bars a(viz);
  for (int gap = a.size() / 2; gap > 0; gap /= 2) {
    for (int i = gap; i < a.size(); ++i) {
      const int key = a.get(i);
      int j = i;
      for (; j >= gap; j -= gap) {
        const int value = a.get(j - gap);
        if (value <= key) break;
        a.set(j, value);
      }
      a.set(j, key);
    }
  }
  return a.finish();
}

void merge_sort_range(Bars& a, int lo, int hi) {  // [lo, hi)
  if (hi - lo < 2) return;
  const int mid = lo + (hi - lo) / 2;
  merge_sort_range(a, lo, mid);
  merge_sort_range(a, mid, hi);
  std::vector<int> left;
  std::vector<int> right;
  for (int i = lo; i < mid; ++i) left.push_back(a.get(i));
  for (int i = mid; i < hi; ++i) right.push_back(a.get(i));
  std::size_t l = 0;
  std::size_t r = 0;
  for (int k = lo; k < hi; ++k) {
    const bool take_left = r == right.size() || (l < left.size() && left[l] <= right[r]);
    a.set(k, take_left ? left[l++] : right[r++]);
  }
}

bool merge_sort(Viz& viz) {
  Bars a(viz);
  merge_sort_range(a, 0, a.size());
  return a.finish();
}

// Lomuto partition around the last element.
void quick_sort_range(Bars& a, int lo, int hi) {  // [lo, hi]
  if (lo >= hi) return;
  int store = lo;
  for (int i = lo; i < hi; ++i) {
    if (a.less(i, hi)) {
      if (i != store) a.swap(i, store);
      ++store;
    }
  }
  if (store != hi) a.swap(store, hi);
  quick_sort_range(a, lo, store - 1);
  quick_sort_range(a, store + 1, hi);
}

bool quick_sort(Viz& viz) {
  Bars a(viz);
  quick_sort_range(a, 0, a.size() - 1);
  return a.finish();
}

void sift_down(Bars& a, int root, int end) {
  while (2 * root + 1 < end) {
    int child = 2 * root + 1;
    if (child + 1 < end && a.less(child, child + 1)) ++child;
    if (!a.less(root, child)) return;
    a.swap(root, child);
    root = child;
  }
}

bool heap_sort(Viz& viz) {
  Bars a(viz);
  for (int start = a.size() / 2 - 1; start >= 0; --start) sift_down(a, start, a.size());
  for (int end = a.size() - 1; end > 0; --end) {
    a.swap(0, end);
    sift_down(a, 0, end);
  }
  return a.finish();
}

// Count each value, prefix-sum the counts into positions, then place stably from the back.
bool counting_sort(Viz& viz) {
  Bars a(viz);
  std::vector<int> input;
  for (int i = 0; i < a.size(); ++i) input.push_back(a.get(i));
  std::vector<int> count(static_cast<std::size_t>(std::ranges::max(input) + 1), 0);
  for (const int v : input) ++count[v];
  for (std::size_t i = 1; i < count.size(); ++i) count[i] += count[i - 1];
  std::vector<int> output(input.size());
  for (const int v : input | std::views::reverse) output[--count[v]] = v;
  for (int i = 0; i < a.size(); ++i) a.set(i, output[i]);
  return a.finish();
}

// Least-significant-digit radix sort: a stable bucket pass per decimal digit.
bool radix_sort(Viz& viz) {
  Bars a(viz);
  int max = 0;
  for (int i = 0; i < a.size(); ++i) max = std::max(max, a.get(i));
  for (int exp = 1; max / exp > 0; exp *= 10) {
    std::vector<std::vector<int>> buckets(10);
    for (int i = 0; i < a.size(); ++i) {
      const int v = a.get(i);
      buckets[static_cast<std::size_t>(v / exp % 10)].push_back(v);
    }
    int k = 0;
    for (const auto& bucket : buckets) {
      for (const int v : bucket) a.set(k++, v);
    }
  }
  return a.finish();
}

}  // namespace

std::vector<Algorithm> sorting_algorithms() {
  return {
      {"bubble-sort", "sorting", "Swap adjacent out-of-order pairs until none remain. O(n²)",
       bubble_sort, 8},
      {"selection-sort", "sorting", "Repeatedly select the minimum of the unsorted tail. O(n²)",
       selection_sort, 8},
      {"insertion-sort", "sorting", "Grow a sorted prefix by inserting one element at a time. O(n²)",
       insertion_sort, 10},
      {"shell-sort", "sorting", "Insertion sort over shrinking gaps. ~O(n^1.3)", shell_sort, 20},
      {"merge-sort", "sorting", "Sort both halves, then merge them. O(n log n)", merge_sort, 20},
      {"quick-sort", "sorting", "Partition around a pivot, recurse on both sides. O(n log n) avg",
       quick_sort, 25},
      {"heap-sort", "sorting", "Build a max-heap, then pop the max to the end. O(n log n)",
       heap_sort, 20},
      {"counting-sort", "sorting", "Count occurrences, then rebuild in order. O(n + k)",
       counting_sort, 40},
      {"radix-sort", "sorting", "Stable bucket pass per decimal digit, least significant first. O(dn)",
       radix_sort, 40},
  };
}

}  // namespace algoviz
