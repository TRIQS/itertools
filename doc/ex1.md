@page ex1 Example 1: Comparison with std::ranges

[TOC]

In this example, we compare **itertools**'s adapted ranges with their counterparts from `std::ranges`.

```cpp
#include "./print.hpp"

#include <itertools/itertools.hpp>
#include <array>
#include <ranges>
#include <vector>

int main() {
  // we use the following vector and array as our base ranges for the examples
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  std::array<char, 2> arr {'a', 'b'};

  // compare itertools::enumerate with std::views::enumerate (only available in c++23 with gcc>=13)
  print_tuple_range(itertools::enumerate(arr));
  print_tuple_range(std::views::enumerate(arr));

  // compare itertools::transform with std::views::transform
  auto square = [](int i) { return i * i; };
  print_simple_range(itertools::transform(vec, square));
  print_simple_range(std::views::transform(vec, square));

  // compare itertools::zip with std::views::zip
  print_tuple_range(itertools::zip(vec, arr));
  print_tuple_range(std::views::zip(vec, arr));

  // compare itertools::product with std::views::cartesian_product (only available in c++23 with gcc>=13)
  print_tuple_range(itertools::product(vec, arr));
  print_tuple_range(std::views::cartesian_product(vec, arr));

  // compare itertools::stride with std::views::stride (only available in c++23 with gcc>=13)
  print_simple_range(itertools::stride(vec, 2));
  print_simple_range(std::views::stride(vec, 2));

  // compare itertools::slice with std::views::counted
  print_simple_range(itertools::slice(vec, 1, 5));
  print_simple_range(std::views::counted(vec.begin() + 1, 4));

  // compare itertools::range with std::views::iota (only available in c++23 with gcc>=13)
  print_simple_range(itertools::range(10, 20, 2));
  print_simple_range(std::views::iota(10, 20) | std::views::stride(2));
}
```

Output:

```
(0, a) (1, b)
(0, a) (1, b)
1 4 9 16 25 36
1 4 9 16 25 36
(1, a) (2, b)
(1, a) (2, b)
(1, a) (1, b) (2, a) (2, b) (3, a) (3, b) (4, a) (4, b) (5, a) (5, b) (6, a) (6, b)
(1, a) (1, b) (2, a) (2, b) (3, a) (3, b) (4, a) (4, b) (5, a) (5, b) (6, a) (6, b)
1 3 5
1 3 5
2 3 4 5
2 3 4 5
10 12 14 16 18
10 12 14 16 18
```