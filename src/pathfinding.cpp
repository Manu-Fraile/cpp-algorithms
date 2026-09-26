#include <array>
#include <climits>
#include <cstdlib>
#include <format>
#include <functional>
#include <optional>
#include <queue>
#include <vector>

#include "algorithms.hpp"
#include "grid.hpp"

namespace algoviz {
namespace {

constexpr int kWidth = 39;
constexpr int kHeight = 19;

// Cheapest cost from start to goal, computed without drawing; nullopt when unreachable.
std::optional<int> reference_cost(const Grid& g, bool weighted) {
  std::vector<int> dist(static_cast<std::size_t>(g.size()), INT_MAX);
  using Entry = std::pair<int, int>;  // cost, cell index
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> queue;
  dist[g.index(*g.start)] = 0;
  queue.emplace(0, g.index(*g.start));
  while (!queue.empty()) {
    const auto [d, i] = queue.top();
    queue.pop();
    if (d > dist[i]) continue;
    for (const Pos n : g.neighbors(g.pos(i))) {
      if (g[n].wall) continue;
      const int next = d + (weighted ? g.cost(n) : 1);
      if (next < dist[g.index(n)]) {
        dist[g.index(n)] = next;
        queue.emplace(next, g.index(n));
      }
    }
  }
  const int goal = dist[g.index(*g.goal)];
  return goal == INT_MAX ? std::nullopt : std::optional(goal);
}

// Random walls (and optionally mud) inside a walled border, retried until the goal is reachable.
Grid make_arena(Viz& viz, bool with_mud) {
  while (true) {
    Grid g(kWidth, kHeight);
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        const bool border = x == 0 || y == 0 || x == kWidth - 1 || y == kHeight - 1;
        g[{x, y}].wall = border || viz.random(0, 99) < 28;
        g[{x, y}].mud = with_mud && viz.random(0, 99) < 25;
      }
    }
    g.start = Pos{2, kHeight / 2};
    g.goal = Pos{kWidth - 3, kHeight / 2};
    g[*g.start] = {};
    g[*g.goal] = {};
    if (reference_cost(g, false)) return g;
  }
}

// Follows `came_from` back from the goal and paints the path. Returns its cost, or nullopt if
// the links do not form a valid walk from the start.
std::optional<int> trace_path(Viz& viz, Grid& g, const std::vector<int>& came_from) {
  std::vector<Pos> path;
  int cost = 0;
  for (Pos p = *g.goal; p != *g.start;) {
    const int parent = came_from[g.index(p)];
    if (parent < 0 || static_cast<int>(path.size()) > g.size()) return std::nullopt;
    const Pos q = g.pos(parent);
    if (g[p].wall || std::abs(p.x - q.x) + std::abs(p.y - q.y) != 1) return std::nullopt;
    cost += g.cost(p);
    path.push_back(p);
    p = q;
  }
  for (auto it = path.rbegin(); it != path.rend(); ++it) {
    g[*it].mark = Mark::path;
    viz.show(g.str(), std::format("path: {} steps, cost {}", path.size(), cost));
  }
  return cost;
}

bool breadth_first_search(Viz& viz) {
  Grid g = make_arena(viz, false);
  std::vector<int> came_from(static_cast<std::size_t>(g.size()), -1);
  std::vector<bool> seen(static_cast<std::size_t>(g.size()), false);
  std::queue<Pos> frontier;
  frontier.push(*g.start);
  seen[g.index(*g.start)] = true;
  for (int expanded = 1; !frontier.empty(); ++expanded) {
    const Pos p = frontier.front();
    frontier.pop();
    if (p == g.goal) break;
    g[p].mark = Mark::visited;
    for (const Pos n : g.neighbors(p)) {
      if (g[n].wall || seen[g.index(n)]) continue;
      seen[g.index(n)] = true;
      came_from[g.index(n)] = g.index(p);
      g[n].mark = Mark::frontier;
      frontier.push(n);
    }
    viz.show(g.str(), std::format("expanded {}   queue {}", expanded, frontier.size()));
  }
  return trace_path(viz, g, came_from) == reference_cost(g, false);
}

bool depth_first_search(Viz& viz) {
  Grid g = make_arena(viz, false);
  std::vector<int> came_from(static_cast<std::size_t>(g.size()), -1);
  std::vector<bool> seen(static_cast<std::size_t>(g.size()), false);
  std::vector<Pos> stack{*g.start};
  seen[g.index(*g.start)] = true;
  for (int expanded = 1; !stack.empty(); ++expanded) {
    const Pos p = stack.back();
    stack.pop_back();
    if (p == g.goal) break;
    g[p].mark = Mark::visited;
    for (const Pos n : g.neighbors(p)) {
      if (g[n].wall || seen[g.index(n)]) continue;
      seen[g.index(n)] = true;
      came_from[g.index(n)] = g.index(p);
      g[n].mark = Mark::frontier;
      stack.push_back(n);
    }
    viz.show(g.str(), std::format("expanded {}   stack {}", expanded, stack.size()));
  }
  // DFS finds *a* path, not the shortest one.
  return trace_path(viz, g, came_from).has_value();
}

// Dijkstra when `use_heuristic` is false; A* (Manhattan distance, admissible since every step
// costs at least 1) when true.
bool best_first(Viz& viz, bool use_heuristic) {
  Grid g = make_arena(viz, true);
  const Pos goal = *g.goal;
  const auto heuristic = [&](Pos p) {
    return use_heuristic ? std::abs(p.x - goal.x) + std::abs(p.y - goal.y) : 0;
  };
  std::vector<int> cost(static_cast<std::size_t>(g.size()), INT_MAX);
  std::vector<int> came_from(static_cast<std::size_t>(g.size()), -1);
  using Entry = std::pair<int, int>;  // priority, cell index
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> open;
  cost[g.index(*g.start)] = 0;
  open.emplace(heuristic(*g.start), g.index(*g.start));
  for (int expanded = 1; !open.empty(); ++expanded) {
    const int i = open.top().second;
    open.pop();
    const Pos p = g.pos(i);
    if (g[p].mark == Mark::visited) continue;  // stale queue entry
    if (p == goal) break;
    g[p].mark = Mark::visited;
    for (const Pos n : g.neighbors(p)) {
      const int next = cost[i] + g.cost(n);
      if (g[n].wall || next >= cost[g.index(n)]) continue;
      cost[g.index(n)] = next;
      came_from[g.index(n)] = i;
      g[n].mark = Mark::frontier;
      open.emplace(next + heuristic(n), g.index(n));
    }
    viz.show(g.str(), std::format("expanded {}   cost so far {}   (~~ mud costs {})", expanded,
                                  cost[i], kMudCost));
  }
  return trace_path(viz, g, came_from) == reference_cost(g, true);
}

bool dijkstra(Viz& viz) { return best_first(viz, false); }
bool a_star(Viz& viz) { return best_first(viz, true); }

// Two BFS waves, one from each end, expanded a full layer at a time until they touch.
bool bidirectional_bfs(Viz& viz) {
  Grid g = make_arena(viz, false);
  const auto n = static_cast<std::size_t>(g.size());
  const std::array<int, 2> roots{g.index(*g.start), g.index(*g.goal)};
  std::array<std::vector<int>, 2> dist{std::vector<int>(n, -1), std::vector<int>(n, -1)};
  std::array<std::vector<int>, 2> parent{std::vector<int>(n, -1), std::vector<int>(n, -1)};
  std::array<std::vector<int>, 2> layer{std::vector<int>{roots[0]}, std::vector<int>{roots[1]}};
  dist[0][roots[0]] = 0;
  dist[1][roots[1]] = 0;

  int best = INT_MAX;
  std::array<int, 2> meet{-1, -1};  // meet[s] is the touching cell reached by side s
  for (std::size_t side = 0; best == INT_MAX && !layer[0].empty() && !layer[1].empty();
       side = 1 - side) {
    const std::size_t other = 1 - side;
    std::vector<int> next;
    for (const int i : layer[side]) {
      g[g.pos(i)].mark = Mark::visited;
      for (const Pos p : g.neighbors(g.pos(i))) {
        const int j = g.index(p);
        if (g[p].wall) continue;
        if (dist[other][j] >= 0 && dist[side][i] + 1 + dist[other][j] < best) {
          best = dist[side][i] + 1 + dist[other][j];
          meet[side] = i;
          meet[other] = j;
        }
        if (dist[side][j] >= 0) continue;
        dist[side][j] = dist[side][i] + 1;
        parent[side][j] = i;
        g[p].mark = Mark::frontier;
        next.push_back(j);
      }
      viz.show(g.str(), std::format("expanding from the {}", side == 0 ? "start" : "goal"));
    }
    layer[side] = std::move(next);
  }
  if (best == INT_MAX) return false;

  // Stitch both halves into one start-to-goal parent chain.
  std::vector<int> came_from(n, -1);
  for (int x = meet[0]; x != roots[0]; x = parent[0][x]) came_from[x] = parent[0][x];
  came_from[meet[1]] = meet[0];
  for (int x = meet[1]; x != roots[1]; x = parent[1][x]) came_from[parent[1][x]] = x;
  const auto cost = trace_path(viz, g, came_from);
  return cost == best && cost == reference_cost(g, false);
}

// Fill the region around a seed cell, stopping at walls.
bool flood_fill(Viz& viz) {
  Grid g(kWidth, kHeight);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) g[{x, y}].wall = viz.random(0, 99) < 35;
  }
  const Pos seed{kWidth / 2, kHeight / 2};
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) g[{seed.x + dx, seed.y + dy}].wall = false;
  }
  std::vector<Pos> stack{seed};
  g[seed].mark = Mark::frontier;
  int filled = 0;
  while (!stack.empty()) {
    const Pos p = stack.back();
    stack.pop_back();
    g[p].mark = Mark::visited;
    ++filled;
    for (const Pos n : g.neighbors(p)) {
      if (g[n].wall || g[n].mark != Mark::none) continue;
      g[n].mark = Mark::frontier;
      stack.push_back(n);
    }
    viz.show(g.str(), std::format("filled {}", filled));
  }
  // The fill is closed: no filled cell touches an open, unfilled one.
  for (int i = 0; i < g.size(); ++i) {
    if (g[g.pos(i)].mark != Mark::visited) continue;
    for (const Pos n : g.neighbors(g.pos(i))) {
      if (!g[n].wall && g[n].mark != Mark::visited) return false;
    }
  }
  return filled > 0;
}

}  // namespace

std::vector<Algorithm> pathfinding_algorithms() {
  return {
      {"bfs", "pathfinding", "Breadth-first search: shortest path on unweighted grids",
       breadth_first_search, 12},
      {"dfs", "pathfinding", "Depth-first search: dives deep, finds a path (rarely the shortest)",
       depth_first_search, 12},
      {"dijkstra", "pathfinding", "Cheapest-first expansion on a weighted grid", dijkstra, 12},
      {"a-star", "pathfinding", "Dijkstra guided by a Manhattan-distance heuristic", a_star, 20},
      {"bidirectional-bfs", "pathfinding", "Two BFS waves meeting in the middle",
       bidirectional_bfs, 15},
      {"flood-fill", "pathfinding", "Paint a connected region, stopping at walls", flood_fill, 12},
  };
}

}  // namespace algoviz
