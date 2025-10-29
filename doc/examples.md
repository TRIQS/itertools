@page examples Examples

[TOC]

- @ref ex1 "Example 1: Comparison with std::ranges"

@section compiling Compiling the examples

All examples have been compiled on a MacBook Pro with an Apple M2 Max chip.
At the point of writing this documentation only gcc has implemented the required `std::ranges` for some of the examples.
We therefore used gcc 15.2.0 together with cmake 4.1.2.

Assuming that the actual example code is in a file `main.cpp` and that the `print.hpp` (see below) is in the same 
directory, the following generic `CMakeLists.txt` should work for all examples:

```cmake
cmake_minimum_required(VERSION 3.20)
project(example CXX)

# set required standard (needed for some std::ranges)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fetch itertools from github
set(Build_Tests OFF CACHE BOOL "" FORCE)
include (FetchContent)
FetchContent_Declare(
  itertools
  GIT_REPOSITORY https://github.com/TRIQS/itertools.git
  GIT_TAG        2.0.x
)
FetchContent_MakeAvailable(itertools)

# build the example
add_executable(ex main.cpp)
target_link_libraries(ex itertools::itertools_c)
```

@subsection print_header print.hpp

To print the elements of a range to `stdout`, we used the following header:

@include print.hpp