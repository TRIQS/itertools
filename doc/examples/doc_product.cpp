#include <itertools/itertools.hpp>
#include <iostream>
#include <vector>

int main() {
  std::vector<int> v1{1, 2, 3};
  std::vector<char> v2{'a', 'b'};
  for (auto [i, c] : itertools::product(v1, v2)) std::cout << "(" << i << ", " << c << ")\n";
}
