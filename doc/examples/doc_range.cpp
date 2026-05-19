#include <itertools/itertools.hpp>
#include <iostream>

int main() {
  for (auto i : itertools::range(5)) std::cout << i << " ";
  std::cout << "\n";

  for (auto i : itertools::range(-2, 1)) std::cout << i << " ";
  std::cout << "\n";

  for (auto i : itertools::range(10, 3, -2)) std::cout << i << " ";
  std::cout << "\n";

  for (auto i : itertools::range(0, 10, -1)) std::cout << i << " ";
  std::cout << "\n";
}
