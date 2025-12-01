#include <itertools/itertools.hpp>
#include <iostream>
#include <list>

int main() {
  std::list<int> list{1, 2, 3, 4, 5};
  for (auto i : itertools::transform(list, [](int i) { return i * i; })) std::cout << i << " ";
  std::cout << "\n";
}
