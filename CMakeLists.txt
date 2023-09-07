cmake_minimum_required(VERSION 3.23)
project(cpp_algorithms)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpp_algorithms
        #data_structures/avltree.cpp
        #sorting/insertion-sort.cpp
        #mathematics/collatz.cpp mathematics/fibonacci.cpp main.cpp
        main.cpp)

add_executable(collatz mathematics/collatz.cpp)
add_executable(goldbach mathematics/goldbach.cpp)
add_executable(fibonacci mathematics/fibonacci.cpp)
add_executable(insertion-sort sorting/insertion-sort.cpp)
add_executable(counting-sort sorting/counting-sort.cpp)
