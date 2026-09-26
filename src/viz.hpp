#pragma once

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace algoviz {

namespace ansi {
inline constexpr std::string_view reset = "\x1b[0m";
inline constexpr std::string_view bold = "\x1b[1m";
inline constexpr std::string_view red = "\x1b[31m";
inline constexpr std::string_view green = "\x1b[32m";
inline constexpr std::string_view yellow = "\x1b[33m";
inline constexpr std::string_view blue = "\x1b[34m";
inline constexpr std::string_view magenta = "\x1b[35m";
inline constexpr std::string_view cyan = "\x1b[36m";
inline constexpr std::string_view white = "\x1b[37m";
inline constexpr std::string_view gray = "\x1b[90m";
inline constexpr std::string_view bg_red = "\x1b[41m";
inline constexpr std::string_view bg_green = "\x1b[42m";
inline constexpr std::string_view bg_yellow = "\x1b[43m";
inline constexpr std::string_view bg_blue = "\x1b[44m";
inline constexpr std::string_view bg_cyan = "\x1b[46m";
inline constexpr std::string_view bg_white = "\x1b[47m";
inline constexpr std::string_view bg_gray = "\x1b[100m";
}  // namespace ansi

// Wraps `text` in a color escape and a reset.
std::string paint(std::string_view color, std::string_view text);

// Everything an algorithm needs to animate itself: where to draw, how fast, and a seeded RNG.
class Viz {
 public:
  Viz(std::ostream& out, std::chrono::milliseconds delay, std::uint32_t seed);

  void set_title(std::string title) { title_ = std::move(title); }

  // Draws one frame (title, body, status line) in place, then waits for the frame delay.
  void show(std::string_view body, std::string_view status = {});

  // Uniform integer in [lo, hi].
  int random(int lo, int hi);
  std::mt19937& rng() { return rng_; }

 private:
  std::ostream& out_;
  std::chrono::milliseconds delay_;
  std::mt19937 rng_;
  std::string title_;
  std::string frame_;
};

// A fixed-size grid of colored glyphs for free-form drawings: trees, graphs, charts.
class Canvas {
 public:
  Canvas(int width, int height);

  void put(int x, int y, std::string_view glyph, std::string_view color = {});
  void text(int x, int y, std::string_view ascii, std::string_view color = {});
  void line(int x0, int y0, int x1, int y1, std::string_view glyph, std::string_view color = {});

  [[nodiscard]] std::string str() const;

 private:
  struct Cell {
    std::string glyph = " ";
    std::string_view color;
  };

  int width_;
  int height_;
  std::vector<Cell> cells_;
};

}  // namespace algoviz
