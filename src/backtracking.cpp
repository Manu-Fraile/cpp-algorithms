#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <format>
#include <numeric>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

// Place one queen per row; undo the last placement whenever a row has no safe column.
bool n_queens(Viz& viz) {
  constexpr int n = 8;
  std::array<int, n> column{};  // column[row] of the queen in that row, -1 if none
  column.fill(-1);
  int placements = 0;

  const auto safe = [&](int row, int col) {
    for (int r = 0; r < row; ++r) {
      const int c = column[static_cast<std::size_t>(r)];
      if (c == col || std::abs(c - col) == row - r) return false;
    }
    return true;
  };
  const auto draw = [&](std::string_view status) {
    std::string body;
    for (int r = 0; r < n; ++r) {
      for (int c = 0; c < n; ++c) {
        const std::string_view square = (r + c) % 2 == 0 ? ansi::bg_gray : ansi::bg_blue;
        const bool queen = column[static_cast<std::size_t>(r)] == c;
        body += paint(square, queen ? paint(ansi::yellow, " Q") + std::string(square) + " " : "   ");
      }
      body += '\n';
    }
    viz.show(body, std::format("{}   placements {}", status, placements));
  };

  const auto solve = [&](const auto& self, int row) -> bool {
    if (row == n) return true;
    for (int col = 0; col < n; ++col) {
      if (!safe(row, col)) continue;
      column[static_cast<std::size_t>(row)] = col;
      ++placements;
      draw(std::format("row {}: queen in column {}", row, col));
      if (self(self, row + 1)) return true;
      column[static_cast<std::size_t>(row)] = -1;
      draw(std::format("row {}: dead end, backtrack", row + 1));
    }
    return false;
  };
  if (!solve(solve, 0)) return false;
  draw("solved");
  for (int r = 0; r < n; ++r) {
    if (!safe(r, column[static_cast<std::size_t>(r)])) return false;
  }
  return true;
}

// Backtracking Sudoku solver that always fills the cell with the fewest candidates first.
bool sudoku(Viz& viz) {
  // A well-known puzzle; its digits are randomly relabelled, which keeps it valid.
  constexpr std::string_view kPuzzle =
      "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
  std::array<int, 10> relabel{};
  std::iota(relabel.begin(), relabel.end(), 0);
  std::shuffle(relabel.begin() + 1, relabel.end(), viz.rng());
  std::array<int, 81> board{};
  for (std::size_t i = 0; i < board.size(); ++i) board[i] = relabel[static_cast<std::size_t>(kPuzzle[i] - '0')];
  const std::array<int, 81> clues = board;
  int steps = 0;

  const auto candidates = [&](std::size_t cell) {
    std::bitset<10> used;
    const std::size_t r = cell / 9;
    const std::size_t c = cell % 9;
    for (std::size_t k = 0; k < 9; ++k) {
      used.set(static_cast<std::size_t>(board[r * 9 + k]));
      used.set(static_cast<std::size_t>(board[k * 9 + c]));
      used.set(static_cast<std::size_t>(board[(r / 3 * 3 + k / 3) * 9 + c / 3 * 3 + k % 3]));
    }
    return ~used & std::bitset<10>(0b1111111110);
  };
  const auto draw = [&](std::size_t current, std::string_view status) {
    std::string body;
    for (std::size_t i = 0; i < 81; ++i) {
      const std::string_view color = i == current ? ansi::yellow : clues[i] ? ansi::white : ansi::cyan;
      body += board[i] ? paint(color, std::format(" {}", board[i])) : paint(ansi::gray, " ·");
      if (i % 9 == 2 || i % 9 == 5) body += paint(ansi::gray, " │");
      if (i % 9 == 8) body += i == 26 || i == 53 ? "\n" + paint(ansi::gray, "───────┼───────┼───────") + '\n' : "\n";
    }
    viz.show(body, std::format("{}   steps {}", status, steps));
  };

  const auto solve = [&](const auto& self) -> bool {
    std::size_t best = 81;
    for (std::size_t i = 0; i < 81; ++i) {
      if (board[i] == 0 && (best == 81 || candidates(i).count() < candidates(best).count())) best = i;
    }
    if (best == 81) return true;
    const std::bitset<10> options = candidates(best);
    for (int d = 1; d <= 9; ++d) {
      if (!options[static_cast<std::size_t>(d)]) continue;
      board[best] = d;
      ++steps;
      draw(best, std::format("try {} ({} option{})", d, options.count(), options.count() == 1 ? "" : "s"));
      if (self(self)) return true;
    }
    board[best] = 0;
    draw(best, "no digit fits: backtrack");
    return false;
  };
  if (!solve(solve)) return false;
  draw(81, "solved");

  for (std::size_t i = 0; i < 81; ++i) {
    if (clues[i] && clues[i] != board[i]) return false;
    const int d = board[i];
    board[i] = 0;
    const bool fits = candidates(i)[static_cast<std::size_t>(d)];
    board[i] = d;
    if (!fits) return false;
  }
  return true;
}

// Move n disks: park n-1 on the spare peg, move the largest, then bring the n-1 back on top.
bool tower_of_hanoi(Viz& viz) {
  constexpr int kDisks = 6;
  std::array<std::vector<int>, 3> pegs;
  for (int d = kDisks; d >= 1; --d) pegs[0].push_back(d);
  int moves = 0;
  bool legal = true;

  const auto draw = [&](std::string_view status) {
    constexpr int kPegWidth = 2 * kDisks + 3;
    Canvas canvas(3 * kPegWidth, kDisks + 2);
    for (int p = 0; p < 3; ++p) {
      const int center = p * kPegWidth + kPegWidth / 2;
      canvas.line(center, 0, center, kDisks, "│", ansi::gray);
      canvas.line(p * kPegWidth, kDisks + 1, (p + 1) * kPegWidth - 2, kDisks + 1, "▔", ansi::gray);
      const auto& peg = pegs[static_cast<std::size_t>(p)];
      for (std::size_t level = 0; level < peg.size(); ++level) {
        const int d = peg[level];
        const int y = kDisks - static_cast<int>(level);
        canvas.line(center - d, y, center + d, y, "█", d % 2 ? ansi::cyan : ansi::magenta);
      }
    }
    viz.show(canvas.str(), std::format("{}   moves {} / {}", status, moves, (1 << kDisks) - 1));
  };

  const auto move = [&](int from, int to) {
    auto& src = pegs[static_cast<std::size_t>(from)];
    auto& dst = pegs[static_cast<std::size_t>(to)];
    legal = legal && !src.empty() && (dst.empty() || dst.back() > src.back());
    dst.push_back(src.back());
    src.pop_back();
    ++moves;
    draw(std::format("disk {}: peg {} → peg {}", dst.back(), from + 1, to + 1));
  };
  const auto hanoi = [&](const auto& self, int n, int from, int to, int spare) -> void {
    if (n == 0) return;
    self(self, n - 1, from, spare, to);
    move(from, to);
    self(self, n - 1, spare, to, from);
  };
  draw("start");
  hanoi(hanoi, kDisks, 0, 2, 1);
  return legal && moves == (1 << kDisks) - 1 && pegs[2].size() == kDisks;
}

}  // namespace

std::vector<Algorithm> backtracking_algorithms() {
  return {
      {"n-queens", "backtracking", "Place 8 queens so none attack each other", n_queens, 40},
      {"sudoku", "backtracking", "Backtracking solver, most-constrained cell first", sudoku, 60},
      {"tower-of-hanoi", "backtracking", "Recursive solution in 2ⁿ - 1 moves", tower_of_hanoi, 180},
  };
}

}  // namespace algoviz
