#include <itertools/itertools.hpp>
#include <array>
#include <iostream>

int main() {
  std::array<int, 5> arr{1, 2, 3, 4, 5};

  for (auto i : itertools::slice(arr, 1, 3)) std::cout << i << " ";
  std::cout << "\n";

  for (auto i : itertools::slice(arr, 3, 7)) std::cout << i << " ";
  std::cout << "\n";

  for (auto i : itertools::slice(arr, 4, 3)) std::cout << i << " ";
  std::cout << "\n";
}
