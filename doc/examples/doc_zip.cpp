#include <itertools/itertools.hpp>
#include <iostream>
#include <vector>

int main() {
  std::vector<int> v1{1, 2, 3};
  std::vector<char> v2{'a', 'b', 'c', 'd', 'e'};

  for (auto [i1, i2] : itertools::zip(v1, v1)) std::cout << "(" << i1 << ", " << i2 << ") ";
  std::cout << "\n";
  
  for (auto [i1, i2, c3] : itertools::zip(v1, v1, v2)) std::cout << "(" << i1 << ", " << i2 << ", " << c3 << ") ";
  std::cout << "\n";
}
