// Copyright (c) 2020-2022 Simons Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors: Nils Wentzell

#include <gtest/gtest.h>
#include <itertools/itertools.hpp>

#include <algorithm>
#include <array>
#include <list>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using namespace itertools;

// A non-copyable integer.
struct non_copyable_int {
  int i;
  non_copyable_int(int u) : i(u){};
  non_copyable_int(non_copyable_int const &) = delete;
  non_copyable_int(non_copyable_int &&)      = default;

  operator int() const { return i; }
  friend std::ostream &operator<<(std::ostream &out, non_copyable_int const &x) { return out << x.i; }
};

TEST(Itertools, Distance) {
  // check that itertools::distance implementation agrees with std::distance
  std::vector<long> vec{1, 2, 3, 4, 5, 6, 8, 9, 10};
  std::list<long> list{1, 2, 3, 4, 5, 6, 8, 9, 10};
  auto it1 = vec.begin() + 2;
  auto it2 = vec.end() - 2;
  auto it3 = std::next(list.begin(), 1);
  auto it4 = std::prev(list.end(), 1);
  EXPECT_EQ(itertools::distance(it1, it2), std::distance(it1, it2));
  EXPECT_EQ(itertools::distance(it3, it4), std::distance(it3, it4));
}

TEST(Itertools, MakeVectorFromRange) {
  // check that we can create a vector from a range
  std::vector<long> vec1{1, 2, 3, 4, 5};
  auto vec2 = make_vector_from_range(range(1, 6));
  EXPECT_EQ(vec1, vec2);
}

TEST(Itertools, ChunkRange) {
  // check the correctness of some chunked ranges
  EXPECT_EQ(chunk_range(0, 10, 1, 0), std::make_pair(0l, 10l));
  EXPECT_EQ(chunk_range(0, 10, 2, 0), std::make_pair(0l, 5l));
  EXPECT_EQ(chunk_range(0, 10, 2, 1), std::make_pair(5l, 10l));
  EXPECT_EQ(chunk_range(0, 10, 3, 0), std::make_pair(0l, 4l));
  EXPECT_EQ(chunk_range(0, 10, 3, 1), std::make_pair(4l, 7l));
  EXPECT_EQ(chunk_range(0, 10, 3, 2), std::make_pair(7l, 10l));
  EXPECT_EQ(chunk_range(0, 10, 4, 0), std::make_pair(0l, 3l));
  EXPECT_EQ(chunk_range(0, 10, 4, 1), std::make_pair(3l, 6l));
  EXPECT_EQ(chunk_range(0, 10, 4, 2), std::make_pair(6l, 8l));
  EXPECT_EQ(chunk_range(0, 10, 4, 3), std::make_pair(8l, 10l));
  EXPECT_EQ(chunk_range(0, 10, 11, 0), std::make_pair(0l, 1l));
  EXPECT_EQ(chunk_range(0, 10, 11, 10), std::make_pair(10l, 10l));
}

TEST(Itertools, Enumerate) {
  std::vector<int> vec{6, 5, 4, 3, 2, 1};

  // enumerate elements of a range
  for (auto [j, x] : enumerate(vec)) {
    EXPECT_TRUE(j + x == 6);
    EXPECT_TRUE(x == vec[j]);
  }

  // enumerate and change elements of the underlying range
  std::vector<int> vec_compare{0, 1, 2, 3, 4, 5};
  for (auto [j, x] : enumerate(vec)) { x = static_cast<int>(j); }
  EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec_compare.begin()));

  // enumerate non-copyable elements of a range
  std::array<non_copyable_int, 6> arr{6, 5, 4, 3, 2, 1};
  for (auto [j, x] : enumerate(arr)) {
    EXPECT_TRUE(6 - j == x);
    EXPECT_TRUE(arr[j] == x);
  }
}

TEST(Itertools, Transform) {
  std::vector<int> vec{1, 2, 3, 4, 5, 6};
  auto square = [](int i) { return i * i; };

  // square elements of an integer range
  int i = 0;
  for (auto x : transform(vec, square)) {
    ++i;
    EXPECT_TRUE(i * i == x);
  }

  // square non-copyable elements of an integer range
  std::array<non_copyable_int, 6> arr{1, 2, 3, 4, 5, 6};
  i = 0;
  for (auto x : transform(arr, square)) {
    ++i;
    EXPECT_TRUE(i * i == x);
  }
}

TEST(Itertools, Zip) {
  std::array<non_copyable_int, 6> arr{6, 5, 4, 3, 2, 1};
  std::vector<int> vec1{1, 2, 3, 4, 5, 6};

  // zip two ranges
  for (auto [x, y] : zip(arr, vec1)) { EXPECT_TRUE(7 - y == x); }

  // change the elements of the second range to be equal to the first
  for (auto [x, y] : zip(arr, vec1)) { y = x.i; }
  EXPECT_TRUE(std::equal(vec1.begin(), vec1.end(), arr.begin()));

  // check that the size of a zipped range is correct
  std::vector<double> vec2{1, 2, 3};
  int count = 0;
  for ([[maybe_unused]] auto [x, y] : zip(arr, vec2)) { ++count; }
  EXPECT_EQ(count, vec2.size());
}

TEST(Itertools, Product) {
  // multiply two ranges and iterate over them
  std::vector<int> vec1{0, 1, 2, 3, 4};
  std::array<non_copyable_int, 5> arr{0, 1, 2, 3, 4};
  int i = 0, j = 0;
  for (auto [x, y] : product(vec1, arr)) {
    EXPECT_EQ(x, i);
    EXPECT_EQ(y, j);
    ++j;
    if (j > arr.back()) {
      ++i;
      j = 0;
    }
    std::cout << "[" << x << "," << y << "]\n";
  }

  // multiply two ranges and change the elements of the second range
  std::vector<int> vec2{1, 2, 3, 4};
  std::vector<int> vec3{1, 1, 1, 1};
  for (auto [x, y] : product(vec2, vec3)) { y *= x; }
  EXPECT_EQ(vec3, std::vector<int>(4, 1 * 2 * 3 * 4));

  // make a product range from an array of ranges
  constexpr int N = 4;
  std::array<range, N> range_arr{range(1), range(2), range(3), range(4)};
  int count = 0;
  for ([[maybe_unused]] auto [u, v, w, x] : make_product(range_arr)) ++count;
  EXPECT_EQ(count, 1 * 2 * 3 * 4);
}

TEST(Itertools, Slice) {
  // slice an integer range in various ways
  for (long N : range(1, 6)) {
    for (auto start_idx : range(N)) {
      for (auto M : range(1, 6)) {
        auto sliced  = slice(range(N), start_idx, M);
        long sum     = std::accumulate(sliced.cbegin(), sliced.cend(), 0l);
        long end_idx = std::max(std::min(M, N), start_idx);
        EXPECT_EQ(sum, end_idx * (end_idx - 1) / 2 - start_idx * (start_idx - 1) / 2);
      }
    }
  }

  // change the elements of a sliced range
  std::vector<int> vec1{0, 1, 2, 3, 4};
  std::vector<int> vec2{0, 0, 0, 3, 4};
  for (auto &x : slice(vec1, 1, 3)) { x = 0; }
  EXPECT_EQ(vec1, vec2);
}

TEST(Itertools, Stride) {
  // check correctness of different strides
  std::vector<int> vec1{0, 1, 2, 3, 4};
  for (int s = 1; s < 6; ++s) {
    int i    = 0;
    int size = 0;
    for (auto x : stride(vec1, s)) {
      EXPECT_EQ(i, x);
      i += s;
      ++size;
    }
    EXPECT_EQ(size, (vec1.size() - 1) / s + 1);
  }

  // check an empty strided range
  std::vector<int> vec2;
  int empty_size = 0;
  for ([[maybe_unused]] auto x : stride(vec2, 2)) { ++empty_size; }
  EXPECT_EQ(empty_size, 0);
}

TEST(Itertools, Range) {
  // check different integer ranges
  int L = 5;
  for (int a = -L; a <= L; a++)
    for (int b = -L; b <= L; b++)
      for (int s = 1; s <= 3; s++) {
        int sum_with_range = 0;
        for (auto i : range(a, b, (a <= b) ? s : -s)) { sum_with_range += static_cast<int>(i); }
        int sum_exact = 0;
        if (a <= b)
          for (int i = a; i < b; i += s) { sum_exact += i; }
        else
          for (int i = a; i > b; i -= s) { sum_exact += i; }
        EXPECT_EQ(sum_with_range, sum_exact);
      }

  // check the size of various valid integer ranges
  EXPECT_EQ(range(1).size(), 1);
  EXPECT_EQ(range(-10, 10, 2).size(), 10);
  EXPECT_EQ(range(10, -10, -2).size(), 10);

  EXPECT_EQ(range(0).size(), 0);
  EXPECT_EQ(range(-1, 0, -3).size(), 0);
  EXPECT_EQ(range(10, -10, 2).size(), 0);
  EXPECT_EQ(range(-10, 10, -2).size(), 0);

  // product of integer ranges
  long res = 0;
  for (auto [i, j, k] : product_range(5, 5, 5)) res += i * j * k;
  EXPECT_EQ(res, 1000);
}

TEST(Itertools, CombinationOfRangeAdaptingFunctions) {
  std::vector<int> vec1{1, 2, 3, 4, 5, 6};
  std::vector<int> vec2{0, 1, 2, 3, 4};

  // imitate enumerate with transform
  auto enum_imitation = [n = 0](auto x) mutable { return std::make_tuple(n++, x); };

  // zip a transformed and an enumerated range
  for (auto [x1, x2] : zip(transform(vec1, enum_imitation), enumerate(vec1))) { EXPECT_TRUE(x1 == x2); }

  // enumerate a transformed range
  for (auto [i, x] : enumerate(transform(vec1, enum_imitation))) { std::cout << i << "  [" << std::get<0>(x) << ", " << std::get<1>(x) << "]\n"; }

  // transform a product range
  auto add = [](auto const &p) {
    auto [v, w] = p;
    return v + w;
  };
  int total = 0;
  for (auto sum : transform(product(vec1, vec1), add)) total += sum;
  EXPECT_EQ(total, 252);

  // slice a zipped range
  for (auto [x1, x2] : slice(zip(vec1, vec1), 0, 4)) { EXPECT_EQ(x1, x2); }

  // take the product of an integer range and a transformed range (sum up number from 0 to 99)
  auto times_ten = [](auto i) { return 10 * i; };
  total          = 0;
  for (auto [a, b] : product(transform(range(10), times_ten), range(10))) { total += static_cast<int>(a + b); }
  EXPECT_EQ(total, 99 * 100 / 2);

  // stride through a product range
  for (int s = 1; s < 6; ++s) {
    int idx = 0;
    for (auto [x1, x2] : stride(product(vec2, vec2), s)) {
      auto i = idx / static_cast<int>(vec2.size());
      auto j = idx - i * static_cast<int>(vec2.size());
      EXPECT_EQ(x1, i);
      EXPECT_EQ(x2, j);
      idx += s;
    }
  }

  // zip two strided ranges
  for (int s = 1; s < 6; ++s) {
    int i = 0;
    for (auto [x1, x2] : zip(stride(vec2, s), stride(vec2, s))) {
      EXPECT_EQ(x1, i);
      EXPECT_EQ(x2, i);
      i += s;
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
