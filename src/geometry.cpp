#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "algorithms.hpp"

namespace algoviz {
namespace {

struct Point {
  int x;
  int y;
  friend auto operator<=>(const Point&, const Point&) = default;
};

// > 0 when o→a→b turns counter-clockwise (y up), < 0 clockwise, 0 when collinear.
int cross(Point o, Point a, Point b) { return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x); }

// Andrew's monotone chain: sweep left to right for the lower hull, then back for the upper hull,
// popping any point that would make a clockwise turn.
bool convex_hull(Viz& viz) {
  constexpr int kWidth = 72;
  constexpr int kHeight = 18;
  std::vector<Point> points;
  for (int i = 0; i < 28; ++i) points.push_back({viz.random(1, kWidth - 2), viz.random(1, kHeight - 2)});
  std::ranges::sort(points);
  const auto [first, last] = std::ranges::unique(points);
  points.erase(first, last);

  std::vector<Point> hull;
  const auto draw = [&](Point candidate, std::string_view status, bool popping) {
    Canvas canvas(kWidth, kHeight);
    for (const Point& p : points) canvas.put(p.x, p.y, "•", ansi::gray);
    for (std::size_t i = 1; i < hull.size(); ++i) {
      canvas.line(hull[i - 1].x, hull[i - 1].y, hull[i].x, hull[i].y, "·", ansi::green);
    }
    if (!hull.empty()) {
      canvas.line(hull.back().x, hull.back().y, candidate.x, candidate.y, "·", popping ? ansi::red : ansi::yellow);
    }
    for (const Point& p : hull) canvas.put(p.x, p.y, "●", ansi::green);
    canvas.put(candidate.x, candidate.y, "●", ansi::yellow);
    viz.show(canvas.str(), std::format("{}   hull size {}", status, hull.size()));
  };
  // Lower pass keeps at least 1 point; upper pass must not pop into the finished lower hull.
  const auto add = [&](Point p, std::size_t keep) {
    while (hull.size() > keep && cross(hull[hull.size() - 2], hull.back(), p) <= 0) {
      draw(p, "clockwise turn: pop", true);
      hull.pop_back();
    }
    hull.push_back(p);
    draw(p, "counter-clockwise turn: keep", false);
  };
  for (const Point& p : points) add(p, 1);
  const std::size_t lower_size = hull.size();
  for (auto it = points.rbegin() + 1; it != points.rend(); ++it) add(*it, lower_size);
  hull.pop_back();  // the start point closes the loop

  // Every point must lie on or left of every hull edge.
  for (std::size_t i = 0; i < hull.size(); ++i) {
    const Point a = hull[i];
    const Point b = hull[(i + 1) % hull.size()];
    if (std::ranges::any_of(points, [&](Point p) { return cross(a, b, p) < 0; })) return false;
  }
  return hull.size() >= 3;
}

}  // namespace

std::vector<Algorithm> geometry_algorithms() {
  return {
      {"convex-hull", "geometry", "Andrew's monotone chain: the rubber band around points",
       convex_hull, 120},
  };
}

}  // namespace algoviz
