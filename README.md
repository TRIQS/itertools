[![build](https://github.com/TRIQS/itertools/workflows/build/badge.svg?branch=notriqs)](https://github.com/TRIQS/itertools/actions?query=workflow%3Abuild)

# itertools

itertools is a single-header C++ library that allows, with a simple interface, for the writing of 
various types of range-based for loops.

## Simple Example

```c++
#include <itertools/itertools.hpp>
#include <vector>

using namespace itertools;

int main(){

  // ===== Ranges of numbers =====

  for(int i: range(10)) { /* 0, 1, .., 9  */ }

  for(int i: range(2, 10, 2)) { /* 2, 4, 6, 8 */ }

  for (auto [i, j] : product_range(5, 5)) {
    /* (0, 0), (0, 1), .. (0, 4),
       (1, 0), (1, 2), .. (1, 4),
       ...
       (4, 0), (4, 2), .. (4, 4) */
  }

  // ===== Adapting ranges =====

  std::vector<char> Vc{'a', 'b', 'c'};

  for (auto [i, c] : enumerate(Vc)) {
    /* (0, 'a'), (1, 'b'), (2, 'c') */
  }

  std::vector<double> Vd{2.0, 4.0, 1.0};

  for (auto [c, d] : zip(Vc, Vd)) {
    /* ('a', 2.0), ('b', 4.0), ('c', 1.0) */
  }

  for (auto [c, d] : product(Vc, Vd)) {
    /* ('a', 2.0), ('a', 4.0), ('a', 1.0),
       ('b', 2.0), ('b', 4.0), ('b', 1.0),
       ('c', 2.0), ('c', 4.0), ('c', 1.0) */
  }

  for (auto x : transform(Vd, [](auto d){ return d * d; })) {
    /* 4.0, 16.0, 1.0 */
  }

}
```

For further examples we refer the users to our [tests](https://github.com/TRIQS/itertools/tree/unstable/test/c++).
