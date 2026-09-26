#include "algorithms.hpp"

#include <algorithm>

namespace algoviz {

const std::vector<Algorithm>& all_algorithms() {
  static const std::vector<Algorithm> all = [] {
    std::vector<Algorithm> out;
    for (auto list : {sorting_algorithms, searching_algorithms, pathfinding_algorithms,
                      maze_algorithms, graph_algorithms, dynamic_programming_algorithms,
                      data_structure_algorithms, mathematics_algorithms, backtracking_algorithms,
                      geometry_algorithms}) {
      std::ranges::copy(list(), std::back_inserter(out));
    }
    return out;
  }();
  return all;
}

const Algorithm* find_algorithm(std::string_view name) {
  const auto& all = all_algorithms();
  const auto it = std::ranges::find(all, name, &Algorithm::name);
  return it == all.end() ? nullptr : &*it;
}

}  // namespace algoviz
