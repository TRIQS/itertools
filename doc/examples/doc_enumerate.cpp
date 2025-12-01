#include <itertools/itertools.hpp>
#include <iostream>
#include <vector>

int main() {
  std::vector<char> vec{'a', 'b', 'c'};
  for (auto [idx, val] : itertools::enumerate(vec)) std::cout << "(" << idx << ", " << val << ")\n";
}
