#include <algorithm>
#include <bit>
#include <cstdlib>
#include <climits>
#include <format>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

// `count` distinct keys from [10, 99], in random order.
std::vector<int> random_keys(Viz& viz, int count) {
  std::vector<int> keys(90);
  std::iota(keys.begin(), keys.end(), 10);
  std::ranges::shuffle(keys, viz.rng());
  keys.resize(static_cast<std::size_t>(count));
  return keys;
}

// Self-balancing binary search tree: after each insert, rotations keep subtree heights within 1.
class AvlTree {
 public:
  explicit AvlTree(Viz& viz) : viz_(viz) {}

  void insert(int key) { insert(root_, key); }

  // True when keys are ordered, cached heights are right, and every node is balanced.
  [[nodiscard]] bool valid(int expected_size) const {
    int count = 0;
    return verified_height(root_.get(), INT_MIN, INT_MAX, count) >= 0 && count == expected_size;
  }

 private:
  struct Node {
    explicit Node(int k) : key(k) {}
    int key;
    int height = 1;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };
  using Tree = std::unique_ptr<Node>;

  static int height(const Tree& t) { return t ? t->height : 0; }
  static void update(Node& n) { n.height = 1 + std::max(height(n.left), height(n.right)); }

  static Tree rotate_right(Tree y) {
    Tree x = std::move(y->left);
    y->left = std::move(x->right);
    update(*y);
    x->right = std::move(y);
    update(*x);
    return x;
  }

  static Tree rotate_left(Tree x) {
    Tree y = std::move(x->right);
    x->right = std::move(y->left);
    update(*x);
    y->left = std::move(x);
    update(*y);
    return y;
  }

  void insert(Tree& node, int key) {
    if (!node) {
      node = std::make_unique<Node>(key);
      draw(std::format("inserted {}", key), key);
      return;
    }
    draw(std::format("insert {}: {} {}", key, key < node->key ? "left of" : "right of", node->key),
         node->key);
    insert(key < node->key ? node->left : node->right, key);
    rebalance(node);
  }

  void rebalance(Tree& node) {
    update(*node);
    const int balance = height(node->left) - height(node->right);
    const int key = node->key;
    if (balance > 1) {
      if (height(node->left->left) < height(node->left->right)) {
        node->left = rotate_left(std::move(node->left));
        draw(std::format("left-right case at {}: rotate its left child left", key), key);
      }
      node = rotate_right(std::move(node));
      draw(std::format("left-heavy at {}: rotate right", key), key);
    } else if (balance < -1) {
      if (height(node->right->right) < height(node->right->left)) {
        node->right = rotate_right(std::move(node->right));
        draw(std::format("right-left case at {}: rotate its right child right", key), key);
      }
      node = rotate_left(std::move(node));
      draw(std::format("right-heavy at {}: rotate left", key), key);
    }
  }

  // Lays the tree out by in-order position; returns the node's column.
  static int place(const Node* node, int depth, int& column, Canvas& canvas, int highlight) {
    if (!node) return -1;
    const int left = place(node->left.get(), depth + 1, column, canvas, highlight);
    const int x = column++ * 4 + 1;
    const int right = place(node->right.get(), depth + 1, column, canvas, highlight);
    const int y = depth * 2;
    canvas.text(x, y, std::format("{:>2}", node->key), node->key == highlight ? ansi::yellow : ansi::cyan);
    if (left >= 0 || right >= 0) {
      for (int i = left >= 0 ? left : x; i <= (right >= 0 ? right : x); ++i) canvas.put(i, y + 1, "─", ansi::gray);
      if (left >= 0) canvas.put(left, y + 1, "┌", ansi::gray);
      if (right >= 0) canvas.put(right, y + 1, "┐", ansi::gray);
      canvas.put(x, y + 1, left >= 0 && right >= 0 ? "┴" : left >= 0 ? "┘" : "└", ansi::gray);
    }
    return x;
  }

  void draw(std::string_view status, int highlight) {
    Canvas canvas(64, 11);
    int column = 0;
    place(root_.get(), 0, column, canvas, highlight);
    viz_.show(canvas.str(), status);
  }

  static int verified_height(const Node* n, int lo, int hi, int& count) {
    if (!n) return 0;
    if (n->key <= lo || n->key >= hi) return -1;
    ++count;
    const int l = verified_height(n->left.get(), lo, n->key, count);
    const int r = verified_height(n->right.get(), n->key, hi, count);
    if (l < 0 || r < 0 || std::abs(l - r) > 1 || n->height != 1 + std::max(l, r)) return -1;
    return n->height;
  }

  Viz& viz_;
  Tree root_;
};

bool avl_tree(Viz& viz) {
  constexpr int kKeys = 15;
  AvlTree tree(viz);
  for (const int key : random_keys(viz, kKeys)) tree.insert(key);
  return tree.valid(kKeys);
}

// Array-backed min-heap: push sifts up, pop moves the last leaf to the root and sifts down.
class MinHeap {
 public:
  explicit MinHeap(Viz& viz) : viz_(viz) {}

  [[nodiscard]] bool empty() const { return a_.empty(); }

  void push(int value) {
    a_.push_back(value);
    std::size_t i = a_.size() - 1;
    draw(i, std::format("push {}", value));
    while (i > 0 && a_[i] < a_[(i - 1) / 2]) {
      std::swap(a_[i], a_[(i - 1) / 2]);
      i = (i - 1) / 2;
      draw(i, "sift up");
    }
  }

  int pop() {
    const int top = a_.front();
    a_.front() = a_.back();
    a_.pop_back();
    popped_.push_back(top);
    std::size_t i = 0;
    draw(i, std::format("pop {}: last leaf moves to the root", top));
    while (true) {
      std::size_t smallest = i;
      for (const std::size_t child : {2 * i + 1, 2 * i + 2}) {
        if (child < a_.size() && a_[child] < a_[smallest]) smallest = child;
      }
      if (smallest == i) return top;
      std::swap(a_[i], a_[smallest]);
      i = smallest;
      draw(i, "sift down");
    }
  }

 private:
  void draw(std::size_t highlight, std::string_view status) {
    constexpr int kWidth = 64;
    Canvas canvas(kWidth, 8);
    for (std::size_t i = 0; i < a_.size(); ++i) {
      const int level = std::bit_width(i + 1) - 1;
      const auto slot = static_cast<int>(i + 1) - (1 << level);
      const int x = (2 * slot + 1) * kWidth / (2 << level) - 1;
      const int y = level * 2;
      if (i > 0) {
        const bool left_child = i % 2 == 1;
        const int parent_slot = slot / 2;
        const int px = (2 * parent_slot + 1) * kWidth / (1 << level) - 1;
        canvas.put((x + px) / 2 + 1, y - 1, left_child ? "╱" : "╲", ansi::gray);
      }
      canvas.text(x, y, std::format("{:>2}", a_[i]), i == highlight ? ansi::yellow : ansi::cyan);
    }
    std::string body = canvas.str() + "\narray:  ";
    for (std::size_t i = 0; i < a_.size(); ++i) {
      body += paint(i == highlight ? ansi::yellow : ansi::white, std::format("{:>3}", a_[i]));
    }
    body += "\npopped: ";
    for (const int v : popped_) body += paint(ansi::green, std::format("{:>3}", v));
    viz_.show(body, status);
  }

  Viz& viz_;
  std::vector<int> a_;
  std::vector<int> popped_;
};

bool binary_heap(Viz& viz) {
  MinHeap heap(viz);
  for (const int key : random_keys(viz, 15)) heap.push(key);
  std::vector<int> out;
  while (!heap.empty()) out.push_back(heap.pop());
  return out.size() == 15 && std::ranges::is_sorted(out);
}

// Open addressing with linear probing: on a collision, try the next slot.
bool hash_table(Viz& viz) {
  constexpr int kSlots = 17;
  std::vector<std::optional<int>> slots(kSlots);
  const auto draw = [&](int probe, std::string_view probe_color, std::string_view status) {
    std::string body;
    for (int i = 0; i < kSlots; ++i) body += paint(ansi::gray, std::format("{:>4}", i));
    body += '\n';
    for (int i = 0; i < kSlots; ++i) {
      const auto& slot = slots[static_cast<std::size_t>(i)];
      body += paint(i == probe ? probe_color : ansi::cyan, slot ? std::format("{:>4}", *slot) : "   ·");
    }
    viz.show(body, status);
  };
  // Returns the slot holding `key`, or the empty slot where it would go.
  const auto probe = [&](int key, std::string_view verb) {
    int i = key % kSlots;
    while (slots[static_cast<std::size_t>(i)] && *slots[static_cast<std::size_t>(i)] != key) {
      draw(i, ansi::red, std::format("{} {}: hash {} → slot {} taken, probe next", verb, key, key % kSlots, i));
      i = (i + 1) % kSlots;
    }
    return i;
  };

  const std::vector<int> keys = random_keys(viz, 13);
  const std::vector<int> stored(keys.begin(), keys.end() - 1);
  const int absent = keys.back();
  for (const int key : stored) {
    const int i = probe(key, "insert");
    slots[static_cast<std::size_t>(i)] = key;
    draw(i, ansi::green, std::format("insert {}: stored in slot {}", key, i));
  }
  bool ok = true;
  for (const int key : stored) {
    const int i = probe(key, "find");
    ok = ok && slots[static_cast<std::size_t>(i)] == key;
    draw(i, ansi::green, std::format("find {}: found in slot {}", key, i));
  }
  const int i = probe(absent, "find");
  draw(i, ansi::yellow, std::format("find {}: hit an empty slot, so it is absent", absent));
  return ok && !slots[static_cast<std::size_t>(i)];
}

}  // namespace

std::vector<Algorithm> data_structure_algorithms() {
  return {
      {"avl-tree", "data-structures", "Self-balancing BST: rotations keep heights within 1",
       avl_tree, 250},
      {"binary-heap", "data-structures", "Min-heap priority queue: sift up on push, down on pop",
       binary_heap, 180},
      {"hash-table", "data-structures", "Open addressing with linear probing", hash_table, 350},
  };
}

}  // namespace algoviz
