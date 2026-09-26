// Runs every algorithm headless (no delay, output discarded) over many seeds and checks that each
// one's self-verification passes.

#include <chrono>
#include <cstdint>
#include <format>
#include <iostream>
#include <set>
#include <streambuf>
#include <string_view>

#include "algorithms.hpp"

namespace {

class NullBuffer : public std::streambuf {
 protected:
  int_type overflow(int_type c) override { return traits_type::not_eof(c); }
  std::streamsize xsputn(const char*, std::streamsize n) override { return n; }
};

constexpr std::uint32_t kSeeds = 25;

}  // namespace

int main() {
  NullBuffer buffer;
  std::ostream discard(&buffer);
  std::set<std::string_view> names;
  int failures = 0;

  for (const algoviz::Algorithm& algorithm : algoviz::all_algorithms()) {
    if (!names.insert(algorithm.name).second) {
      std::cerr << std::format("duplicate name: {}\n", algorithm.name);
      ++failures;
    }
    for (std::uint32_t seed = 1; seed <= kSeeds; ++seed) {
      algoviz::Viz viz(discard, std::chrono::milliseconds(0), seed);
      if (!algorithm.run(viz)) {
        std::cerr << std::format("FAIL {} (seed {})\n", algorithm.name, seed);
        ++failures;
      }
    }
  }

  std::cout << std::format("{} algorithms x {} seeds: {} failure(s)\n", names.size(), kSeeds, failures);
  return failures == 0 ? 0 : 1;
}
