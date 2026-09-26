#include <algorithm>
#include <format>
#include <vector>

#include "algorithms.hpp"
#include "disjoint_sets.hpp"
#include "grid.hpp"

namespace algoviz {
namespace {

// Odd sizes so rooms sit on odd coordinates, with walls (and pillars) in between.
constexpr int kWidth = 39;
constexpr int kHeight = 19;
constexpr int kRooms = (kWidth / 2) * (kHeight / 2);

Grid solid_grid() {
  Grid g(kWidth, kHeight);
  for (int i = 0; i < g.size(); ++i) g[g.pos(i)].wall = true;
  return g;
}

Pos between(Pos a, Pos b) { return {(a.x + b.x) / 2, (a.y + b.y) / 2}; }

// A perfect maze connects every room with exactly rooms - 1 passages: no loops, no islands.
bool is_perfect_maze(const Grid& g) {
  int open = 0;
  for (int i = 0; i < g.size(); ++i) open += g[g.pos(i)].wall ? 0 : 1;
  std::vector<bool> seen(static_cast<std::size_t>(g.size()), false);
  std::vector<Pos> stack{{1, 1}};
  seen[g.index({1, 1})] = true;
  int reached = 0;
  while (!stack.empty()) {
    const Pos p = stack.back();
    stack.pop_back();
    ++reached;
    for (const Pos n : g.neighbors(p)) {
      if (g[n].wall || seen[g.index(n)]) continue;
      seen[g.index(n)] = true;
      stack.push_back(n);
    }
  }
  return reached == open && open == 2 * kRooms - 1;
}

// Random depth-first walk that backs up whenever it hits a dead end.
bool recursive_backtracker(Viz& viz) {
  Grid g = solid_grid();
  std::vector<Pos> stack{{1, 1}};
  g[{1, 1}] = {.wall = false, .mark = Mark::frontier};
  while (!stack.empty()) {
    const Pos p = stack.back();
    std::vector<Pos> options;
    for (const Pos n : g.neighbors(p, 2)) {
      if (g[n].wall) options.push_back(n);
    }
    if (options.empty()) {
      stack.pop_back();
      g[p].mark = Mark::none;
      if (!stack.empty()) g[between(p, stack.back())].mark = Mark::none;
    } else {
      const Pos next = options[static_cast<std::size_t>(viz.random(0, static_cast<int>(options.size()) - 1))];
      g[between(p, next)] = {.wall = false, .mark = Mark::frontier};
      g[next] = {.wall = false, .mark = Mark::frontier};
      stack.push_back(next);
    }
    viz.show(g.str(), std::format("stack depth {}", stack.size()));
  }
  return is_perfect_maze(g);
}

// Randomized Prim: grow one tree by attaching a random frontier room to it.
bool prim_maze(Viz& viz) {
  Grid g = solid_grid();
  std::vector<Pos> frontier;
  const auto carve = [&](Pos p) {
    g[p] = {.wall = false};
    for (const Pos n : g.neighbors(p, 2)) {
      if (g[n].wall && g[n].mark != Mark::frontier) {
        g[n].mark = Mark::frontier;
        frontier.push_back(n);
      }
    }
  };
  carve({1, 1});
  while (!frontier.empty()) {
    const auto k = static_cast<std::size_t>(viz.random(0, static_cast<int>(frontier.size()) - 1));
    const Pos p = frontier[k];
    frontier[k] = frontier.back();
    frontier.pop_back();
    std::vector<Pos> carved;
    for (const Pos n : g.neighbors(p, 2)) {
      if (!g[n].wall) carved.push_back(n);
    }
    const Pos from = carved[static_cast<std::size_t>(viz.random(0, static_cast<int>(carved.size()) - 1))];
    g[between(p, from)].wall = false;
    carve(p);
    viz.show(g.str(), std::format("frontier {}", frontier.size()));
  }
  return is_perfect_maze(g);
}

// Randomized Kruskal: knock down walls in random order unless the rooms are already connected.
bool kruskal_maze(Viz& viz) {
  Grid g = solid_grid();
  std::vector<Pos> walls;
  for (int y = 1; y < kHeight - 1; ++y) {
    for (int x = 1; x < kWidth - 1; ++x) {
      if (x % 2 == 1 && y % 2 == 1) g[{x, y}].wall = false;  // a room
      if ((x + y) % 2 == 1) walls.push_back({x, y});           // a wall between two rooms
    }
  }
  std::ranges::shuffle(walls, viz.rng());
  DisjointSets rooms(g.size());
  int passages = 0;
  for (const Pos w : walls) {
    const bool vertical = w.x % 2 == 1;  // rooms above and below
    const Pos a = vertical ? Pos{w.x, w.y - 1} : Pos{w.x - 1, w.y};
    const Pos b = vertical ? Pos{w.x, w.y + 1} : Pos{w.x + 1, w.y};
    if (!rooms.unite(g.index(a), g.index(b))) continue;
    g[w].wall = false;
    viz.show(g.str(), std::format("passages {} / {}", ++passages, kRooms - 1));
  }
  return is_perfect_maze(g);
}

}  // namespace

std::vector<Algorithm> maze_algorithms() {
  return {
      {"maze-backtracker", "mazes", "Recursive backtracker: random DFS that backs up at dead ends",
       recursive_backtracker, 12},
      {"maze-prim", "mazes", "Randomized Prim: grow one tree from a random frontier", prim_maze, 15},
      {"maze-kruskal", "mazes", "Randomized Kruskal: merge rooms with union-find", kruskal_maze, 15},
  };
}

}  // namespace algoviz
