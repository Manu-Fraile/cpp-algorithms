#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace algoviz {

struct Pos {
  int x = 0;
  int y = 0;
  friend bool operator==(const Pos&, const Pos&) = default;
};

enum class Mark : std::uint8_t { none, frontier, visited, path };

inline constexpr int kMudCost = 5;

struct Cell {
  bool wall = false;
  bool mud = false;  // costs kMudCost to enter instead of 1
  Mark mark = Mark::none;
};

// A 2D board shared by path finding, maze generation and flood fill. Each cell draws 2 chars wide.
class Grid {
 public:
  Grid(int width, int height);

  [[nodiscard]] int width() const { return width_; }
  [[nodiscard]] int height() const { return height_; }
  [[nodiscard]] int size() const { return width_ * height_; }
  [[nodiscard]] bool contains(Pos p) const;
  [[nodiscard]] int index(Pos p) const { return p.y * width_ + p.x; }
  [[nodiscard]] Pos pos(int index) const { return {index % width_, index / width_}; }

  Cell& operator[](Pos p) { return cells_[static_cast<std::size_t>(index(p))]; }
  const Cell& operator[](Pos p) const { return cells_[static_cast<std::size_t>(index(p))]; }

  // In-bounds orthogonal neighbours `step` cells away.
  [[nodiscard]] std::vector<Pos> neighbors(Pos p, int step = 1) const;
  // Cost of stepping onto `p`.
  [[nodiscard]] int cost(Pos p) const { return (*this)[p].mud ? kMudCost : 1; }

  [[nodiscard]] std::string str() const;

  std::optional<Pos> start;
  std::optional<Pos> goal;

 private:
  int width_;
  int height_;
  std::vector<Cell> cells_;
};

}  // namespace algoviz
