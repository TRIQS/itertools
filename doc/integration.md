@page integration Integration in C++ projects

[TOC]

**itertools** is a header only library.
To use it in your own `C++` code, you simply have to include the following header

```cpp
#include <itertools/itertools.hpp>

// use itertools
```

and tell your compiler/build system where it can find the necessary files.
In the following, we describe some common ways to achieve this (with special focus on CMake).

@section cmake CMake

@subsection fetch FetchContent

If you use [CMake](https://cmake.org/) to build your source code, it is recommended to fetch the source code directly from the
[Github repository](https://github.com/TRIQS/itertools) using CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)
module:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

# fetch from github
include (FetchContent)
FetchContent_Declare(
  itertools
  GIT_REPOSITORY https://github.com/TRIQS/itertools.git
  GIT_TAG        1.2.x
)
FetchContent_MakeAvailable(itertools)

# declare a target and link to itertools
add_executable(my_executable main.cpp)
target_link_libraries(my_executable itertools::itertools_c)
```

Note that the above will also build [goolgetest](https://github.com/google/googletest) and the unit tests for **itertools**.
To disable this, you can put `set(Build_Tests OFF CACHE BOOL "" FORCE)` before fetching the content.

@subsection find_package find_package

If you have already installed **itertools** on your system by following the instructions from the @ref installation page, you can also make
use of CMake's [find_package](https://cmake.org/cmake/help/latest/command/find_package.html) command.
This has the advantage that you don't need to download anything, i.e. no internet connection is required.

Let's assume that **itertools** has been installed to `path_to_install_dir`.
Then linking your project to **itertools** with CMake is as easy as

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

# find itertools
find_package(itertools REQUIRED)

# declare a target and link to itertools
add_executable(my_executable main.cpp)
target_link_libraries(my_executable itertools::itertools_c)
```

In case, CMake cannot find the package, you might have to tell it where to look for the `itertools-config.cmake` file by setting the variable
`itertools_DIR` to `path_to_install_dir/lib/cmake/itertools`.

@subsection add_sub add_subdirectory

You can also integrate **itertools** into our CMake project by placing the entire source tree in a subdirectory and call `add_subdirectory()`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

# add itertools subdirectory
add_subdirectory(deps/itertools)

# declare a target and link to itertools
add_executable(my_executable main.cpp)
target_link_libraries(my_executable itertools::itertools_c)
```

Here, it is assumed that the **itertools** source tree is in a subdirectory `deps/itertools` relative to your `CMakeLists.txt` file.

@section other Other

Since **itertools** is header-only, you can also simply copy the relevant files directly into our project.
For example, you could place the `c++/itertools` directory from the **itertools** source tree into the include path of your project.
You can then build or compile it with any available method.
