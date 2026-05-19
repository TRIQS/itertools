#include <itertools/itertools.hpp>
#include <iostream>

int main() {
  for (auto [i1, i2] : itertools::product_range(2, 3)) std::cout << "(" << i1 << ", " << i2 << ")\n";
}
