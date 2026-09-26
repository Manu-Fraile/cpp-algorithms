#pragma once

#include <numeric>
#include <vector>

namespace algoviz {

// Union-find with path halving. Small inputs here, so no union-by-rank.
class DisjointSets {
 public:
  explicit DisjointSets(int size) : parent_(static_cast<std::size_t>(size)) {
    std::iota(parent_.begin(), parent_.end(), 0);
  }

  int find(int x) {
    while (parent_[x] != x) {
      parent_[x] = parent_[parent_[x]];
      x = parent_[x];
    }
    return x;
  }

  // Returns false when `a` and `b` were already in the same set.
  bool unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return false;
    parent_[b] = a;
    return true;
  }

 private:
  std::vector<int> parent_;
};

}  // namespace algoviz
