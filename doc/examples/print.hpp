#include <iostream>
#include <tuple>
#include <utility>

// helper function to pretty-print a tuple/array
template <typename T, size_t... I> void print_impl(const T &tup, std::index_sequence<I...>) {
  std::cout << "(";
  (..., (std::cout << (I == 0 ? "" : ", ") << std::get<I>(tup)));
  std::cout << ")";
}

// print a tuple/array to std::cout
template <typename T> void print_tuple(const T &tup) { print_impl(tup, std::make_index_sequence<std::tuple_size_v<T>>()); }

// print a range of tuple elements to std::cout
template <typename R> void print_tuple_range(R &&rg) { // NOLINT (cppcoreguidelines-missing-std-forward)
  for (auto &&x : rg) {
    print_tuple(x);
    std::cout << " ";
  }
  std::cout << std::endl;
}

// print a range of simple elements to std::cout
template <typename R> void print_simple_range(R &&rg) { // NOLINT (cppcoreguidelines-missing-std-forward)
  for (auto &&x : rg) { std::cout << x << " "; }
  std::cout << std::endl;
}
