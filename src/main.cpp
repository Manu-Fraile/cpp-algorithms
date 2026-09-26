#include <charconv>
#include <chrono>
#include <format>
#include <iostream>
#include <optional>
#include <random>
#include <string_view>
#include <thread>
#include <vector>

#include "algorithms.hpp"

namespace {

using namespace algoviz;

constexpr std::string_view kUsage = R"(usage:
  algoviz                         list every algorithm
  algoviz <name>... [options]     animate one or more algorithms
  algoviz all [options]           animate all of them, one after another

options:
  --delay <ms>   milliseconds per frame (default: tuned per algorithm)
  --seed <n>     random seed, to replay the exact same run
)";

void list_algorithms() {
  std::string_view category;
  for (const Algorithm& a : all_algorithms()) {
    if (a.category != category) {
      category = a.category;
      std::cout << '\n' << paint(ansi::bold, category) << '\n';
    }
    std::cout << std::format("  {:<20} {}\n", a.name, a.summary);
  }
  std::cout << std::format("\n{} algorithms\n\n{}", all_algorithms().size(), kUsage);
}

std::optional<int> parse_int(std::string_view text) {
  int value = 0;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size() || value < 0) return std::nullopt;
  return value;
}

bool run(const Algorithm& algorithm, std::optional<int> delay_ms, std::uint32_t seed) {
  Viz viz(std::cout, std::chrono::milliseconds(delay_ms.value_or(algorithm.frame_ms)), seed);
  viz.set_title(std::format("{} › {} — {}", algorithm.category, algorithm.name, algorithm.summary));
  std::cout << "\x1b[2J";
  const bool ok = algorithm.run(viz);
  std::cout << (ok ? paint(ansi::green, "✓ self-check passed")
                   : paint(ansi::red, "✗ self-check FAILED"))
            << std::format("   (seed {})\n", seed);
  return ok;
}

}  // namespace

int main(int argc, char** argv) {
  const std::vector<std::string_view> args(argv + 1, argv + argc);
  std::optional<int> delay_ms;
  std::optional<int> seed;
  std::vector<const Algorithm*> selected;

  for (std::size_t i = 0; i < args.size(); ++i) {
    const std::string_view arg = args[i];
    if (arg == "-h" || arg == "--help") {
      std::cout << kUsage;
      return 0;
    }
    if (arg == "--delay" || arg == "--seed") {
      const auto value = i + 1 < args.size() ? parse_int(args[++i]) : std::nullopt;
      if (!value) {
        std::cerr << std::format("{} needs a non-negative number\n", arg);
        return 2;
      }
      (arg == "--delay" ? delay_ms : seed) = value;
    } else if (arg == "all") {
      for (const Algorithm& a : all_algorithms()) selected.push_back(&a);
    } else if (const Algorithm* a = find_algorithm(arg)) {
      selected.push_back(a);
    } else {
      std::cerr << std::format("unknown algorithm '{}'; run algoviz with no arguments to list them\n", arg);
      return 2;
    }
  }

  if (selected.empty()) {
    list_algorithms();
    return 0;
  }

  const auto base_seed = seed ? static_cast<std::uint32_t>(*seed) : std::random_device{}();
  bool all_ok = true;
  for (std::size_t i = 0; i < selected.size(); ++i) {
    if (i > 0) std::this_thread::sleep_for(std::chrono::seconds(1));
    all_ok = run(*selected[i], delay_ms, base_seed + static_cast<std::uint32_t>(i)) && all_ok;
  }
  return all_ok ? 0 : 1;
}
