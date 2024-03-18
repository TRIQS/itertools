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
 * @brief Provides a range adapting function for slicing a given range/view.
 */

#ifndef _ITERTOOLS_SLICE_HPP
#define _ITERTOOLS_SLICE_HPP

#include "./utils.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

namespace itertools {

  namespace detail {

    /**
     * @brief Represents a sliced range.
     * 
     * @details See itertools::slice(R &&, std::ptrdiff_t, std::ptrdiff_t) for more details.
     * 
     * @tparam R Range type.
     */
    template <typename R> struct sliced {
      /// Original range.
      R rg;

      /// Index at which the sliced range starts.
      std::ptrdiff_t start_idx;

      /// Index at which the sliced range ends.
      std::ptrdiff_t end_idx;

      /// Iterator type of the sliced range.
      using iterator = decltype(std::begin(rg));

      /// Const iterator type of the sliced range.
      using const_iterator = decltype(std::cbegin(rg));

      /// Default equal-to operator.
      [[nodiscard]] bool operator==(sliced const &) const = default;

      /**
       * @brief Helper function to calculate the size of the sliced range.
       * @return Number of elements in the slice.
       */
      [[nodiscard]] std::ptrdiff_t size() const {
        std::ptrdiff_t total_size = distance(std::cbegin(rg), std::cend(rg));
        return std::min(total_size, end_idx) - start_idx;
      }

      /**
       * @brief Beginning of the sliced range.
       * @return Iterator to the beginning of the sliced range.
       */
      [[nodiscard]] iterator begin() noexcept { return std::next(std::begin(rg), start_idx); }

      /// Const version of begin().
      [[nodiscard]] const_iterator cbegin() const noexcept { return std::next(std::cbegin(rg), start_idx); }

      /// Const overload of begin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the sliced range.
       * @return Iterator to the end of the sliced range.
       */
      [[nodiscard]] iterator end() noexcept { return std::next(begin(), size()); }

      /// Const version of end().
      [[nodiscard]] const_iterator cend() const noexcept { return std::next(cbegin(), size()); }

      /// Const overload of end().
      [[nodiscard]] const_iterator end() const noexcept { return cend(); }
    };

  } // namespace detail

  /**
   * @brief Lazy-slice a given range.
   *
   * @details Only the part of the given range between the `start_idx` and the `end_idx` is taken into account.
   * If `end_idx` is bigger than the size of the original range, the slice ends at the end of the original range.
   * If `end_idx` is smaller than `start_idx`, the slice is empty. Note that the behaviour is undefined if 
   * `start_idx` is smaller than zero. This function returns an iterable lazy object, which can be used in 
   * range-based for loops:
   * 
   * @code{.cpp}
   * std::array<int, 5> arr { 1, 2, 3, 4, 5 };
   * 
   * for (auto i : slice(arr, 1, 3)) { 
   *     std::cout << i << " ";
   * }
   * std::cout << "\n";
   * 
   * for (auto i : slice(arr, 3, 7)) { 
   *     std::cout << i << " ";
   * }
   * std::cout << "\n";
   * 
   * for (auto i : slice(arr, 4, 3)) { 
   *     std::cout << i << " "; // empty slice
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * 2 3 
   * 4 5
   * ```
   * 
   * @tparam R Range type.
   * @param rg Range to be sliced.
   * @param start_idx Index where the slice starts.
   * @param end_idx Index of the first element past the end of the sliced range (excluded).
   * @return A detail::sliced range.
   */
  template <typename R> [[nodiscard]] detail::sliced<R> slice(R &&rg, std::ptrdiff_t start_idx, std::ptrdiff_t end_idx) {
    return {std::forward<R>(rg), start_idx, std::max(start_idx, end_idx)};
  }

} // namespace itertools

#endif // _ITERTOOLS_SLICE_HPP