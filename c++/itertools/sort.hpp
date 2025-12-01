// Copyright (c) 2024 Simons Foundation
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
// Authors: Thomas Hahn, Olivier Parcollet, Nils Wentzell, chuffa

/**
 * @file
 * @brief Provides functions for sorting ranges.
 */

#ifndef _ITERTOOLS_SORT_HPP
#define _ITERTOOLS_SORT_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>

namespace itertools {

  /**
   * @addtogroup sorting
   * @{
   */

  /**
   * @brief Bubble sort elements in the given range.
   *
   * @details Sort the elements in the range `[first, last)` in the order prescribed by the comparison function `comp`.
   * The underlying sorting algorithm is a stable bubble sort, i.e. already sorted elements will not be swapped. The
   * number of swaps necessary to get the elements into sorted order is recorded and returned.
   *
   * Computational complexity: \f$ \mathcal{O}(n^2) \f$.
   *
   * This function is eager and puts the range in sorted order.
   *
   * @tparam ForwardIt Forward iterator type.
   * @tparam Compare Comparison function type.
   * @param first Forward iterator to the first element of the range.
   * @param last Forward iterator to the element after the last of the range.
   * @param comp Comparison function callable with two dereferenced iterators.
   * @return Number of swaps necessary to sort the range.
   */
  template <std::forward_iterator ForwardIt, class Compare = std::less<>>
  std::size_t bubble_sort(ForwardIt first, ForwardIt last, Compare comp = {}) {
    if (first == last) { return 0; }
    std::size_t n_swaps = 0;
    for (ForwardIt sorted = first; first != last; last = sorted) {
      sorted = first;
      for (ForwardIt curr = first, prev = first; ++curr != last; ++prev) {
        if (comp(*curr, *prev)) {
          std::iter_swap(curr, prev);
          sorted = curr;
          ++n_swaps;
        }
      }
    }
    return n_swaps;
  }

  /**
   * @brief Insertion sort elements in the given range.
   *
   * @details Sort the elements in the range `[first, last)` in the order prescribed by the comparison function `comp`.
   * The underlying sorting algorithm is a stable insertion sort, i.e. already sorted elements will not be swapped. The
   * number of swaps necessary to get the elements into sorted order is recorded and returned.
   *
   * Computational complexity: \f$ \mathcal{O}(n^2) \f$.
   *
   * This function is eager and puts the range in sorted order.
   *
   * @tparam BidirIt Bidirectional iterator type.
   * @tparam Compare Comparison function type.
   * @param first Bidirectional iterator to the first element of the range.
   * @param last Bidirectional iterator to the element after the last of the range.
   * @param comp Comparison function callable with two dereferenced iterators.
   * @return Number of swaps necessary to sort the range.
   */
  template <std::bidirectional_iterator BidirIt, class Compare = std::less<>>
  std::size_t insertion_sort(BidirIt first, BidirIt last, Compare comp = {}) {
    if (first == last) { return 0; }
    std::size_t swaps = 0;
    for (BidirIt i = std::next(first); i != last; ++i) {
      for (BidirIt j = i; j != first && comp(*j, *std::prev(j)); --j) {
        std::iter_swap(std::prev(j), j);
        ++swaps;
      }
    }
    return swaps;
  }

  /** @} */

} // namespace itertools

#endif // _ITERTOOLS_SORT_HPP
