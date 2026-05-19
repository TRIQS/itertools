#include <itertools/itertools.hpp>
#include <iostream>
#include <vector>

int main() {
  std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  for (auto i : itertools::stride(vec, 3)) { std::cout << i << " "; }
  std::cout << "\n";

  for (auto i : itertools::stride(vec, 10)) { std::cout << i << " "; }
  std::cout << "\n";
}
