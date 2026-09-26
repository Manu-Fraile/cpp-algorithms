#include <algorithm>
#include <array>
#include <climits>
#include <cstdlib>
#include <format>
#include <numeric>
#include <queue>
#include <string>
#include <vector>

#include "algorithms.hpp"
#include "disjoint_sets.hpp"

namespace algoviz {
namespace {

struct Node {
  char label;
  int x;
  int y;
};

struct Edge {
  int from;
  int to;
  int weight;
};

enum class Paint { idle, active, chosen, rejected };

constexpr std::array<Node, 8> kNodes{{{'A', 2, 3},
                                      {'B', 18, 1},
                                      {'C', 36, 1},
                                      {'D', 54, 3},
                                      {'E', 6, 11},
                                      {'F', 24, 8},
                                      {'G', 42, 11},
                                      {'H', 58, 13}}};
constexpr int kNodeCount = static_cast<int>(kNodes.size());
constexpr std::array<std::pair<int, int>, 12> kLinks{
    {{0, 1}, {0, 4}, {1, 2}, {1, 5}, {2, 3}, {2, 5}, {2, 6}, {3, 6}, {3, 7}, {4, 5}, {5, 6}, {6, 7}}};
constexpr int kInfinity = INT_MAX;

// The fixed layout with random weights in [lo, hi]. When `directed`, each link points along a
// random node ranking, which always yields a DAG.
std::vector<Edge> random_edges(Viz& viz, int lo, int hi, bool directed) {
  std::array<int, kNodeCount> rank{};
  std::iota(rank.begin(), rank.end(), 0);
  if (directed) std::ranges::shuffle(rank, viz.rng());
  std::vector<Edge> edges;
  for (const auto& [a, b] : kLinks) {
    const bool forward = rank[static_cast<std::size_t>(a)] < rank[static_cast<std::size_t>(b)];
    edges.push_back({forward ? a : b, forward ? b : a, viz.random(lo, hi)});
  }
  return edges;
}

std::string_view color_of(Paint paint) {
  switch (paint) {
    case Paint::active: return ansi::yellow;
    case Paint::chosen: return ansi::green;
    case Paint::rejected: return ansi::red;
    case Paint::idle: break;
  }
  return ansi::gray;
}

std::string_view arrow(int dx, int dy) {
  if (std::abs(dx) >= 3 * std::abs(dy)) return dx > 0 ? "→" : "←";
  if (std::abs(dy) * 3 > std::abs(dx) * 2 && std::abs(dx) < 4) return dy > 0 ? "↓" : "↑";
  if (dx > 0) return dy > 0 ? "↘" : "↗";
  return dy > 0 ? "↙" : "↖";
}

std::string draw_graph(const std::vector<Edge>& edges, const std::vector<Paint>& edge_paint,
                       const std::vector<std::string_view>& node_color, bool directed,
                       bool weighted = true) {
  Canvas canvas(62, 14);
  for (std::size_t k = 0; k < edges.size(); ++k) {
    const Node& a = kNodes[static_cast<std::size_t>(edges[k].from)];
    const Node& b = kNodes[static_cast<std::size_t>(edges[k].to)];
    const std::string_view color = color_of(edge_paint[k]);
    canvas.line(a.x, a.y, b.x, b.y, "·", color);
    if (directed) {
      canvas.put(a.x + (b.x - a.x) * 4 / 5, a.y + (b.y - a.y) * 4 / 5, arrow(b.x - a.x, b.y - a.y),
                 color);
    }
    if (weighted) canvas.text((a.x + b.x) / 2, (a.y + b.y) / 2, std::to_string(edges[k].weight), color);
  }
  for (std::size_t v = 0; v < kNodes.size(); ++v) {
    canvas.text(kNodes[v].x - 1, kNodes[v].y, std::format("({})", kNodes[v].label), node_color[v]);
  }
  return canvas.str();
}

std::string format_distances(const std::vector<int>& dist) {
  std::string out;
  for (std::size_t v = 0; v < dist.size(); ++v) {
    out += dist[v] == kInfinity ? std::format("{}:∞  ", kNodes[v].label)
                                : std::format("{}:{}  ", kNodes[v].label, dist[v]);
  }
  return out;
}

// Kahn's algorithm: repeatedly output a node with no remaining incoming edges.
bool topological_sort(Viz& viz) {
  const std::vector<Edge> edges = random_edges(viz, 1, 9, true);
  std::vector<Paint> paint(edges.size(), Paint::idle);
  std::vector<std::string_view> color(kNodes.size(), ansi::white);
  std::vector<int> indegree(kNodes.size(), 0);
  for (const Edge& e : edges) ++indegree[e.to];
  std::queue<int> ready;
  for (int v = 0; v < kNodeCount; ++v) {
    if (indegree[v] == 0) {
      ready.push(v);
      color[v] = ansi::cyan;
    }
  }
  std::vector<int> order;
  std::string order_text;
  while (!ready.empty()) {
    const int v = ready.front();
    ready.pop();
    order.push_back(v);
    order_text += std::format("{} ", kNodes[static_cast<std::size_t>(v)].label);
    color[v] = ansi::green;
    for (std::size_t k = 0; k < edges.size(); ++k) {
      if (edges[k].from != v) continue;
      paint[k] = Paint::chosen;
      if (--indegree[edges[k].to] == 0) {
        ready.push(edges[k].to);
        color[edges[k].to] = ansi::cyan;
      }
    }
    viz.show(draw_graph(edges, paint, color, true, false),
             std::format("order: {}  (cyan = no incoming edges left)", order_text));
  }
  std::vector<int> position(kNodes.size(), -1);
  for (std::size_t i = 0; i < order.size(); ++i) position[order[i]] = static_cast<int>(i);
  return order.size() == kNodes.size() &&
         std::ranges::all_of(edges, [&](const Edge& e) { return position[e.from] < position[e.to]; });
}

// Relax every edge |V| - 1 times; handles negative weights.
bool bellman_ford(Viz& viz) {
  const std::vector<Edge> edges = random_edges(viz, -3, 9, true);
  // Start from a node with no incoming edges so most of the graph is reachable.
  int source = 0;
  while (std::ranges::any_of(edges, [&](const Edge& e) { return e.to == source; })) ++source;
  std::vector<int> dist(kNodes.size(), kInfinity);
  std::vector<int> via(kNodes.size(), -1);  // edge that last improved each node
  dist[source] = 0;
  std::vector<Paint> paint(edges.size(), Paint::idle);
  std::vector<std::string_view> color(kNodes.size(), ansi::white);
  color[source] = ansi::green;

  for (int round = 1; round < kNodeCount; ++round) {
    bool changed = false;
    for (std::size_t k = 0; k < edges.size(); ++k) {
      const Edge& e = edges[k];
      paint[k] = Paint::active;
      std::string status = std::format("round {}: relax {}→{}", round,
                                       kNodes[static_cast<std::size_t>(e.from)].label,
                                       kNodes[static_cast<std::size_t>(e.to)].label);
      if (dist[e.from] != kInfinity && dist[e.from] + e.weight < dist[e.to]) {
        dist[e.to] = dist[e.from] + e.weight;
        via[e.to] = static_cast<int>(k);
        changed = true;
        status += " — improved";
      }
      viz.show(draw_graph(edges, paint, color, true) + '\n' + format_distances(dist), status);
      paint[k] = Paint::idle;
    }
    if (!changed) break;
  }
  for (const int k : via) {
    if (k >= 0) paint[k] = Paint::chosen;
  }
  viz.show(draw_graph(edges, paint, color, true) + '\n' + format_distances(dist),
           "shortest-path tree in green");

  // Optimality: no edge can still be relaxed, and every tree edge is tight.
  for (const Edge& e : edges) {
    if (dist[e.from] != kInfinity && dist[e.from] + e.weight < dist[e.to]) return false;
  }
  for (std::size_t v = 0; v < kNodes.size(); ++v) {
    if (via[v] < 0) continue;
    const Edge& e = edges[static_cast<std::size_t>(via[v])];
    if (dist[e.from] + e.weight != dist[v]) return false;
  }
  return true;
}

// All-pairs shortest paths: allow each node k in turn as an intermediate stop.
bool floyd_warshall(Viz& viz) {
  const std::vector<Edge> edges = random_edges(viz, 1, 9, false);
  const auto n = kNodes.size();
  std::vector<std::vector<int>> dist(n, std::vector<int>(n, kInfinity));
  for (std::size_t v = 0; v < n; ++v) dist[v][v] = 0;
  for (const Edge& e : edges) {
    dist[e.from][e.to] = e.weight;
    dist[e.to][e.from] = e.weight;
  }
  const std::vector<Paint> idle(edges.size(), Paint::idle);

  const auto draw = [&](std::size_t k, std::size_t i, std::size_t j, std::string_view status) {
    std::vector<std::string_view> color(n, ansi::white);
    color[i] = ansi::yellow;
    color[j] = ansi::yellow;
    color[k] = ansi::cyan;
    std::string body = draw_graph(edges, idle, color, false) + "\n    ";
    for (const Node& node : kNodes) body += std::format("{:>4}", node.label);
    for (std::size_t r = 0; r < n; ++r) {
      body += std::format("\n{:>4}", kNodes[r].label);
      for (std::size_t c = 0; c < n; ++c) {
        const std::string cell = dist[r][c] == kInfinity ? "   ∞" : std::format("{:>4}", dist[r][c]);
        const bool via_k = (r == i && c == k) || (r == k && c == j);
        body += paint(r == i && c == j ? ansi::yellow : via_k ? ansi::cyan : ansi::gray, cell);
      }
    }
    viz.show(body, status);
  };

  for (std::size_t k = 0; k < n; ++k) {
    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = 0; j < n; ++j) {
        if (i == j || i == k || j == k) continue;
        std::string status = std::format("via {}: {}→{}", kNodes[k].label, kNodes[i].label, kNodes[j].label);
        if (dist[i][k] != kInfinity && dist[k][j] != kInfinity && dist[i][k] + dist[k][j] < dist[i][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          status += " — shorter";
        }
        draw(k, i, j, status);
      }
    }
  }
  for (std::size_t k = 0; k < n; ++k) {
    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = 0; j < n; ++j) {
        if (dist[i][j] == kInfinity || dist[i][j] > dist[i][k] + dist[k][j]) return false;
      }
    }
  }
  return true;
}

int kruskal_weight(const std::vector<Edge>& edges) {
  std::vector<Edge> sorted = edges;
  std::ranges::sort(sorted, {}, &Edge::weight);
  DisjointSets sets(kNodeCount);
  int total = 0;
  for (const Edge& e : sorted) {
    if (sets.unite(e.from, e.to)) total += e.weight;
  }
  return total;
}

// Take edges cheapest-first, skipping any that would close a cycle.
bool kruskal_mst(Viz& viz) {
  const std::vector<Edge> edges = random_edges(viz, 1, 9, false);
  std::vector<std::size_t> order(edges.size());
  std::iota(order.begin(), order.end(), 0);
  std::ranges::stable_sort(order, {}, [&](std::size_t k) { return edges[k].weight; });
  std::vector<Paint> paint(edges.size(), Paint::idle);
  const std::vector<std::string_view> color(kNodes.size(), ansi::white);
  DisjointSets sets(kNodeCount);
  int total = 0;
  int picked = 0;
  for (const std::size_t k : order) {
    paint[k] = Paint::active;
    viz.show(draw_graph(edges, paint, color, false), std::format("weight {}: consider", edges[k].weight));
    const bool joins = sets.unite(edges[k].from, edges[k].to);
    paint[k] = joins ? Paint::chosen : Paint::rejected;
    if (joins) {
      total += edges[k].weight;
      ++picked;
    }
    viz.show(draw_graph(edges, paint, color, false),
             std::format("{}   tree weight {}", joins ? "added" : "would close a cycle", total));
  }
  return picked == kNodeCount - 1 && total == kruskal_weight(edges);
}

// Grow one tree from A, always adding the cheapest edge leaving it.
bool prim_mst(Viz& viz) {
  const std::vector<Edge> edges = random_edges(viz, 1, 9, false);
  std::vector<Paint> paint(edges.size(), Paint::idle);
  std::vector<std::string_view> color(kNodes.size(), ansi::white);
  std::vector<bool> in_tree(kNodes.size(), false);
  using Entry = std::pair<int, std::size_t>;  // weight, edge index
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> candidates;
  const auto add = [&](int v) {
    in_tree[v] = true;
    color[v] = ansi::green;
    for (std::size_t k = 0; k < edges.size(); ++k) {
      const Edge& e = edges[k];
      if ((e.from == v || e.to == v) && paint[k] == Paint::idle) {
        paint[k] = Paint::active;
        candidates.emplace(e.weight, k);
      }
    }
  };
  add(0);
  int total = 0;
  int picked = 0;
  while (!candidates.empty()) {
    const std::size_t k = candidates.top().second;
    candidates.pop();
    const Edge& e = edges[k];
    if (in_tree[e.from] && in_tree[e.to]) {
      paint[k] = Paint::rejected;
      viz.show(draw_graph(edges, paint, color, false), "both ends already in the tree");
      continue;
    }
    paint[k] = Paint::chosen;
    total += e.weight;
    ++picked;
    add(in_tree[e.from] ? e.to : e.from);
    viz.show(draw_graph(edges, paint, color, false),
             std::format("cheapest leaving edge: {}   tree weight {}", e.weight, total));
  }
  return picked == kNodeCount - 1 && total == kruskal_weight(edges);
}

}  // namespace

std::vector<Algorithm> graph_algorithms() {
  return {
      {"topological-sort", "graphs", "Kahn's algorithm: order a DAG so every edge points forward",
       topological_sort, 700},
      {"bellman-ford", "graphs", "Single-source shortest paths with negative weights", bellman_ford,
       120},
      {"floyd-warshall", "graphs", "All-pairs shortest paths via intermediate nodes", floyd_warshall,
       40},
      {"kruskal-mst", "graphs", "Minimum spanning tree: cheapest edges that avoid cycles",
       kruskal_mst, 350},
      {"prim-mst", "graphs", "Minimum spanning tree grown from one node", prim_mst, 450},
  };
}

}  // namespace algoviz
