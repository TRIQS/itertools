// Copyright (c) 2019-2022 Simons Foundation
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
// Authors: Olivier Parcollet, Nils Wentzell, chuffa

/**
 * @file
 * @brief Provides some utility functions for itertools.
 */

#ifndef _ITERTOOLS_UTILS_HPP
#define _ITERTOOLS_UTILS_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace itertools {

  /**
   * @addtogroup utilities
   * @{
   */

  /**
   * @brief Calculate the distance between two iterators.
   *
   * @details It is similar to <a href="https://en.cppreference.com/w/cpp/iterator/distance">std::distance</a>,
   * except that it can be used for two different iterator types, e.g. in case one of them is a const iterator.
   *
   * @tparam Iter1 Iterator type #1.
   * @tparam Iter2 Iterator type #2.
   * @param first Iterator #1.
   * @param last Iterator #2.
   * @return Number of elements between the two iterators.
   */
  template <typename Iter1, typename Iter2>
  [[nodiscard]] inline typename std::iterator_traits<Iter1>::difference_type distance(Iter1 first, Iter2 last) {
    // O(1) for random access iterators
    if constexpr (std::is_same_v<typename std::iterator_traits<Iter1>::iterator_category, std::random_access_iterator_tag>) {
      return last - first;
    } else { // O(n) for other iterators
      typename std::iterator_traits<Iter1>::difference_type r(0);
      for (; first != last; ++first) ++r;
      return r;
    }
  }

  /**
   * @brief Create a vector from a range.
   *
   * @tparam R Range type.
   * @param rg Range.
   * @return std::vector<T> containing the elements of the range, where T denotes the value type of the range.
   */
  template <typename R> [[nodiscard]] auto make_vector_from_range(R const &rg) {
    std::vector<std::decay_t<decltype(*(std::begin(rg)))>> vec{};
    // do we really want to reserve memory here? maybe only for random access ranges?
    if constexpr (std::is_same_v<decltype(std::cbegin(rg)), decltype(std::cend(rg))>) {
      auto total_size = distance(std::cbegin(rg), std::cend(rg));
      vec.reserve(total_size);
    }
    for (auto const &x : rg) vec.emplace_back(x);
    return vec;
  }

  /**
   * @brief Given an integer range `[first, last)`, divide it as equally as possible into N chunks.
   *
   * @details It is intended to divide a range among different processes. If the size of the range is not
   * divisible by N without a remainder, i.e. `r = (last - first) % N`, then the first `r` chunks have one more element.
   *
   * @param first First value of the range.
   * @param last Last value of the range (excluded).
   * @param n_chunks Number of chunks to divide the range into.
   * @param rank Rank of the calling process.
   * @return Pair of indices specifying the first and last (excluded) value of the chunk assigned to the calling process.
   */
  [[nodiscard]] inline std::pair<std::ptrdiff_t, std::ptrdiff_t> chunk_range(std::ptrdiff_t first, std::ptrdiff_t last, long n_chunks, long rank) {
    auto total_size    = last - first;
    auto chunk_size    = total_size / n_chunks;
    auto n_large_nodes = total_size - n_chunks * chunk_size;
    if (rank < n_large_nodes)
      return {first + rank * (chunk_size + 1), first + (rank + 1) * (chunk_size + 1)};
    else
      return {first + n_large_nodes + rank * chunk_size, first + n_large_nodes + (rank + 1) * chunk_size};
  }

  /** @} */

} // namespace itertools

#endif // _ITERTOOLS_UTILS_HPP