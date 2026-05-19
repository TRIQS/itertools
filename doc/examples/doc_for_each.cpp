#include <itertools/itertools.hpp>
#include <iostream>

int main() {
  // print out the first 10 squares
  itertools::foreach(itertools::range(1, 11), [](int i) { std::cout << i * i << " "; });
  std::cout << "\n";
}
