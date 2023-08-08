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

#include <array>
#include <vector>
#include <numeric>

using namespace itertools;

// An uncopyable int
struct _int {
  int i;
  _int(int u) : i(u){};
  _int(_int const &) = delete;
  _int(_int &&)      = default;

  operator int() const { return i; }
  friend std::ostream &operator<<(std::ostream &out, _int const &x) { return out << x.i; }
};

TEST(Itertools, Zip) {

  std::array<_int, 6> V1{6, 5, 4, 3, 2, 1};
  std::vector<int> V2{1, 2, 3, 4, 5, 6};

  // Zip both vectors and sum
  for (auto [x, y] : zip(V1, V2)) { EXPECT_TRUE(7 - y == x); }

  // Change the values of the second vector to the first
  for (auto [x, y] : zip(V1, V2)) { y = x.i; }
  EXPECT_TRUE(std::equal(V2.begin(), V2.end(), V1.begin()));
}

TEST(Itertools, Enumerate) {

  std::vector<int> V{6, 5, 4, 3, 2, 1};

  // Enumerate the elements of the vector
  for (auto [j, x] : enumerate(V)) {
    EXPECT_TRUE(j + x == 6);
    EXPECT_TRUE(x == V[j]);
  }

  // Enumerate and change values
  std::vector<int> V_compare{0, 1, 2, 3, 4, 5};
  for (auto [j, x] : enumerate(V)) { x = j; }
  EXPECT_TRUE(std::equal(V.begin(), V.end(), V_compare.begin()));

  std::array<_int, 6> W{6, 5, 4, 3, 2, 1};

  // Enumerate the elements of the array
  for (auto [j, x] : enumerate(W)) {
    EXPECT_TRUE(6 - j == x);
    EXPECT_TRUE(W[j] == x);
  }
}

TEST(Itertools, Transform) {

  std::vector<int> V{1, 2, 3, 4, 5, 6};

  // Square the elements of the vector
  auto l = [](int i) { return i * i; };

  int i = 0;
  for (auto x : transform(V, l)) {
    ++i;
    EXPECT_TRUE(i * i == x);
  }

  std::array<_int, 6> W{1, 2, 3, 4, 5, 6};

  // Square the elements of the array
  i = 0;
  for (auto x : transform(W, l)) {
    ++i;
    EXPECT_TRUE(i * i == x);
  }
}

TEST(Itertools, Product) {

  std::vector<int> V1{0, 1, 2, 3, 4};
  std::array<_int, 5> V2{0, 1, 2, 3, 4};
  for (auto [x, y] : product(V1, V2)) { std::cout << "[" << x << "," << y << "]\n"; }

  // Check that we can alter the values
  std::vector<int> V3{1, 2, 3, 4};
  std::vector<int> V4{1, 1, 1, 1};
  for (auto [x, y] : product(V3, V4)) { y *= x; }
  EXPECT_EQ(V4, std::vector<int>(4, 1 * 2 * 3 * 4));
}

TEST(Itertools, Slice) {

  for (long N : range(1, 6)) {
    for (auto start_idx : range(N)) {
      for (auto M : range(1, 6)) {
        auto sliced  = slice(range(N), start_idx, M);
        long sum     = std::accumulate(sliced.cbegin(), sliced.cend(), 0);
        long end_idx = std::max(std::min(M, N), start_idx);
        EXPECT_EQ(sum, end_idx * (end_idx - 1) / 2 - start_idx * (start_idx - 1) / 2);
      }
    }
  }
}

TEST(Itertools, Make_Product) {

  constexpr int N = 4;

  std::array<range, N> range_arr{range(1), range(2), range(3), range(4)};

  int count = 0;
  for (auto [i, j, k, l] : make_product(range_arr)) ++count;

  EXPECT_EQ(count, 1 * 2 * 3 * 4);
}

TEST(Itertools, Multi) {

  std::vector<int> V{1, 2, 3, 4, 5, 6};

  // Build enumerate from transform and compare
  auto l = [n = 0](auto x) mutable { return std::make_tuple(n++, x); };

  for (auto [x1, x2] : zip(transform(V, l), enumerate(V))) { EXPECT_TRUE(x1 == x2); }

  // Chain enumerate and transform
  for (auto [i, x] : enumerate(transform(V, l))) { std::cout << i << "  [" << std::get<0>(x) << ", " << std::get<1>(x) << "]\n"; }

  // Combine transform and product
  auto add = [](auto &&p) {
    auto [v, w] = p;
    return v + w;
  };
  int total = 0;
  for (auto sum : transform(product(V, V), add)) total += sum;
  EXPECT_EQ(total, 252);

  // slice and zip
  for (auto [x1, x2] : slice(zip(V, V), 0, 4)) { EXPECT_EQ(x1, x2); }

  // product and transform
  // Sum up numbers from 0 to 99 in a complicated way..
  auto times_ten = [](auto i) { return 10 * i; };
  total          = 0;
  for (auto [a, b] : product(transform(range(10), times_ten), range(10))) { total += a + b; }
  EXPECT_EQ(total, 99 * 100 / 2);
}

TEST(Itertools, Range) {

  int L = 5;
  for (int a = -L; a <= L; a++)
    for (int b = -L; b <= L; b++)
      for (int s = 1; s <= 3; s++) {

        int sum_with_range = 0;
        for (auto i : range(a, b, (a <= b) ? s : -s)) { sum_with_range += i; }

        int sum_exact = 0;
        if (a <= b)
          for (int i = a; i < b; i += s) { sum_exact += i; }
        else
          for (int i = a; i > b; i -= s) { sum_exact += i; }

        EXPECT_EQ(sum_with_range, sum_exact);
      }

  EXPECT_EQ(range(1).size(), 1);
  EXPECT_EQ(range(-10, 10, 2).size(), 10);
  EXPECT_EQ(range(10, -10, -2).size(), 10);

  EXPECT_EQ(range(0).size(), 0);
  EXPECT_EQ(range(-1, 0, -3).size(), 0);
  EXPECT_EQ(range(10, -10, 2).size(), 0);
  EXPECT_EQ(range(-10, 10, -2).size(), 0);
}

TEST(Itertools, Product_Range) {

  long res = 0;
  for (auto [i, j, k] : product_range(5, 5, 5)) res += i * j * k;
  EXPECT_EQ(res, 1000);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
