# cpp-algorithms

40 classic algorithms, animated in your terminal. Modern C++20 with no dependencies.

```
algoviz dijkstra
algoviz quick-sort --delay 5
algoviz all
```

Every algorithm draws itself frame by frame with ANSI colors, then **checks its own result**.
For example, sorts verify the output is sorted, and path finders compare their path against a
reference Dijkstra. The test suite runs all 40 of them headless across many random seeds.

## Build

Requires CMake ≥ 3.20 and a C++20 compiler (GCC 13+, Clang 17+, Apple Clang 15+, MSVC 19.30+).

```sh
cmake -S . -B build
cmake --build build
./build/algoviz            # list everything
ctest --test-dir build     # run the self-checks
```

Options: `--delay <ms>` overrides the per-frame delay, and `--seed <n>` replays an exact run.
Configure with `-DALGOVIZ_SANITIZE=ON` for ASan+UBSan, or `-DALGOVIZ_WERROR=ON` for strict warnings.

## Algorithms

| Category | Algorithms |
|---|---|
| Sorting | bubble, selection, insertion, shell, merge, quick, heap, counting, radix |
| Searching | linear, binary, Knuth-Morris-Pratt |
| Path finding | BFS, DFS, Dijkstra, A*, bidirectional BFS, flood fill |
| Mazes | recursive backtracker, randomized Prim, randomized Kruskal |
| Graphs | topological sort (Kahn), Bellman-Ford, Floyd-Warshall, Kruskal MST, Prim MST |
| Dynamic programming | Fibonacci (memoized), longest common subsequence, edit distance, 0/1 knapsack |
| Data structures | AVL tree, binary heap, hash table (linear probing) |
| Mathematics | sieve of Eratosthenes, Collatz, Goldbach |
| Backtracking | N-queens, Sudoku, Tower of Hanoi |
| Geometry | convex hull (Andrew's monotone chain) |

## Layout

```
src/viz.{hpp,cpp}     Viz (frame output, delay, RNG) and Canvas (free-form drawing)
src/grid.{hpp,cpp}    the 2D board shared by path finding and mazes
src/<category>.cpp    one file per category; each algorithm is a plain function
src/algorithms.*      the registry
src/main.cpp          the CLI
tests/                runs every algorithm headless and checks its self-verification
```

To add an algorithm, write a `bool name(Viz&)` function in its category file, call
`viz.show(...)` whenever something interesting happens, return whether the result checks out,
and add a line to that file's list.
