#pragma once

#include <string_view>
#include <vector>

#include "viz.hpp"

namespace algoviz {

struct Algorithm {
  std::string_view name;
  std::string_view category;
  std::string_view summary;
  // Animates the algorithm on `viz` and returns whether its result passed a self-check.
  bool (*run)(Viz& viz);
  // Default frame delay; small animations run slower so they stay watchable.
  int frame_ms;
};

std::vector<Algorithm> sorting_algorithms();
std::vector<Algorithm> searching_algorithms();
std::vector<Algorithm> pathfinding_algorithms();
std::vector<Algorithm> maze_algorithms();
std::vector<Algorithm> graph_algorithms();
std::vector<Algorithm> dynamic_programming_algorithms();
std::vector<Algorithm> data_structure_algorithms();
std::vector<Algorithm> mathematics_algorithms();
std::vector<Algorithm> backtracking_algorithms();
std::vector<Algorithm> geometry_algorithms();

// Every algorithm, grouped by category.
const std::vector<Algorithm>& all_algorithms();

// nullptr when no algorithm has that name.
const Algorithm* find_algorithm(std::string_view name);

}  // namespace algoviz
