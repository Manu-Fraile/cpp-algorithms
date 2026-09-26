cmake_minimum_required(VERSION 3.20)
project(algoviz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(ALGOVIZ_WERROR "Treat warnings as errors" OFF)
option(ALGOVIZ_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(algoviz_core STATIC
  src/algorithms.cpp
  src/backtracking.cpp
  src/data_structures.cpp
  src/dynamic_programming.cpp
  src/geometry.cpp
  src/graphs.cpp
  src/grid.cpp
  src/mathematics.cpp
  src/mazes.cpp
  src/pathfinding.cpp
  src/searching.cpp
  src/sorting.cpp
  src/viz.cpp
)
target_include_directories(algoviz_core PUBLIC src)

if(MSVC)
  target_compile_options(algoviz_core PUBLIC /W4 /permissive- /utf-8 $<$<BOOL:${ALGOVIZ_WERROR}>:/WX>)
else()
  target_compile_options(algoviz_core PUBLIC
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Wnon-virtual-dtor -Wold-style-cast
    $<$<BOOL:${ALGOVIZ_WERROR}>:-Werror>)
  if(ALGOVIZ_SANITIZE)
    target_compile_options(algoviz_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(algoviz_core PUBLIC -fsanitize=address,undefined)
  endif()
endif()

add_executable(algoviz src/main.cpp)
target_link_libraries(algoviz PRIVATE algoviz_core)

include(CTest)
if(BUILD_TESTING)
  add_executable(algoviz_tests tests/test_algorithms.cpp)
  target_link_libraries(algoviz_tests PRIVATE algoviz_core)
  add_test(NAME algorithms COMMAND algoviz_tests)
endif()
