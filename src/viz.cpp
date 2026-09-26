#include "viz.hpp"

#include <cstdlib>
#include <ostream>
#include <thread>

namespace algoviz {

std::string paint(std::string_view color, std::string_view text) {
  std::string out;
  out.reserve(color.size() + text.size() + ansi::reset.size());
  out.append(color).append(text).append(ansi::reset);
  return out;
}

Viz::Viz(std::ostream& out, std::chrono::milliseconds delay, std::uint32_t seed)
    : out_(out), delay_(delay), rng_(seed) {}

void Viz::show(std::string_view body, std::string_view status) {
  // Move the cursor home and overdraw, erasing line tails ("\x1b[K"): no flicker from clearing.
  frame_.assign("\x1b[H");
  frame_.append(paint(ansi::bold, title_)).append("\x1b[K\n\x1b[K\n");
  for (std::size_t begin = 0; begin < body.size();) {
    std::size_t end = body.find('\n', begin);
    if (end == std::string_view::npos) end = body.size();
    frame_.append(body.substr(begin, end - begin)).append("\x1b[K\n");
    begin = end + 1;
  }
  frame_.append("\x1b[K\n").append(status).append("\x1b[K\n\x1b[J");
  out_ << frame_ << std::flush;
  if (delay_.count() > 0) std::this_thread::sleep_for(delay_);
}

int Viz::random(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng_); }

Canvas::Canvas(int width, int height)
    : width_(width), height_(height), cells_(static_cast<std::size_t>(width * height)) {}

void Canvas::put(int x, int y, std::string_view glyph, std::string_view color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  Cell& cell = cells_[static_cast<std::size_t>(y * width_ + x)];
  cell.glyph = glyph;
  cell.color = color;
}

void Canvas::text(int x, int y, std::string_view ascii, std::string_view color) {
  for (const char& c : ascii) put(x++, y, std::string_view(&c, 1), color);
}

// Bresenham's line algorithm.
void Canvas::line(int x0, int y0, int x1, int y1, std::string_view glyph, std::string_view color) {
  const int dx = std::abs(x1 - x0);
  const int dy = -std::abs(y1 - y0);
  const int sx = x0 < x1 ? 1 : -1;
  const int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;
  while (true) {
    put(x0, y0, glyph, color);
    if (x0 == x1 && y0 == y1) return;
    const int doubled = 2 * error;
    if (doubled >= dy) {
      error += dy;
      x0 += sx;
    }
    if (doubled <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

std::string Canvas::str() const {
  std::string out;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const Cell& cell = cells_[static_cast<std::size_t>(y * width_ + x)];
      out += cell.color.empty() ? cell.glyph : paint(cell.color, cell.glyph);
    }
    out += '\n';
  }
  return out;
}

}  // namespace algoviz
