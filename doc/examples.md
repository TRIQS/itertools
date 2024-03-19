@page examples Examples

[TOC]

- @ref ex1 "Example 1: Comparison with std::ranges"

@section compiling Compiling the examples

All examples have been compiled on a MacBook Pro with an Apple M2 Max chip.
At the point of writing this documentation only gcc-13 has implemented the required `std::ranges` for some of the examples.
We therefore used gcc 13.2.0 together with cmake 3.27.2.

Assuming that the actual example code is in a file `main.cpp` and that the `print.hpp` (see below) is in the same directory, the following generic `CMakeLists.txt` should work for all examples:

```cmake
cmake_minimum_required(VERSION 3.20)
project(example CXX)

# set required standard (needed for some std::ranges)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fetch itertools from github
set(Build_Tests OFF CACHE BOOL "" FORCE)
include (FetchContent)
FetchContent_Declare(
  itertools
  GIT_REPOSITORY https://github.com/TRIQS/itertools.git
  GIT_TAG        1.2.x
)
FetchContent_MakeAvailable(itertools)

# build the example
add_executable(ex main.cpp)
target_link_libraries(ex itertools::itertools_c)
```

@subsection print_header print.hpp

To print the elements of a range to `stdout`, we used the following header:

```cpp
#include <iostream>
#include <tuple>
#include <utility>

// helper function to pretty-print a tuple/array
template <typename T, size_t... I>
void print_impl(const T& tup, std::index_sequence<I...>) {
  std::cout << "(";
  (..., (std::cout << (I == 0? "" : ", ") << std::get<I>(tup)));
  std::cout << ")";
}

// print a tuple/array to std::cout
template <typename T>
void print_tuple(const T& tup) {
  print_impl(tup, std::make_index_sequence<std::tuple_size_v<T>>());
}

// print a range of tuple elements to std::cout
template <typename R>
void print_tuple_range(R&& rg) {
  for (auto&& x : rg) {
      print_tuple(x);
      std::cout << " ";
  }
  std::cout << std::endl;
}

// print a range of simple elements to std::cout
template <typename R>
void print_simple_range(R&& rg) {
  for (auto&& x : rg) {
      std::cout << x << " ";
  }
  std::cout << std::endl;
}
```