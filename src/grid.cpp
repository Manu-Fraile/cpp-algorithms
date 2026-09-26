#include "grid.hpp"

#include "viz.hpp"

namespace algoviz {

Grid::Grid(int width, int height)
    : width_(width), height_(height), cells_(static_cast<std::size_t>(width * height)) {}

bool Grid::contains(Pos p) const { return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_; }

std::vector<Pos> Grid::neighbors(Pos p, int step) const {
  std::vector<Pos> out;
  for (const Pos n : {Pos{p.x + step, p.y}, Pos{p.x, p.y + step}, Pos{p.x - step, p.y},
                      Pos{p.x, p.y - step}}) {
    if (contains(n)) out.push_back(n);
  }
  return out;
}

std::string Grid::str() const {
  std::string out;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const Pos p{x, y};
      const Cell& cell = (*this)[p];
      std::string_view background;
      if (p == start) {
        background = ansi::bg_green;
      } else if (p == goal) {
        background = ansi::bg_red;
      } else if (cell.wall) {
        background = ansi::bg_white;
      } else if (cell.mark == Mark::path) {
        background = ansi::bg_yellow;
      } else if (cell.mark == Mark::frontier) {
        background = ansi::bg_cyan;
      } else if (cell.mark == Mark::visited) {
        background = ansi::bg_blue;
      }
      const std::string_view glyph = cell.mud && !cell.wall ? "~~" : "  ";
      if (!background.empty()) {
        out += paint(background, glyph);
      } else if (cell.mud) {
        out += paint(ansi::yellow, glyph);
      } else {
        out += glyph;
      }
    }
    out += '\n';
  }
  return out;
}

}  // namespace algoviz
