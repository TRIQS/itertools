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
 * @brief Provides a range adapting function for striding through a given range/view.
 */

#ifndef _ITERTOOLS_STRIDE_HPP
#define _ITERTOOLS_STRIDE_HPP

#include "./iterator_facade.hpp"
#include "./utils.hpp"

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace itertools {

  namespace detail {

    /**
     * @brief Iterator for a detail::strided range.
     * 
     * @details It stores an iterator of the original range as well as a stride. Incrementing advances the original 
     * iterator by the given stride. Dereferencing simply returns the dereferenced original iterator.
     * 
     * See itertools::stride(R &&, std::ptrdiff_t) for more details.
     * 
     * @tparam Iter Iterator type.
     */
    template <typename Iter> struct stride_iter : iterator_facade<stride_iter<Iter>, typename std::iterator_traits<Iter>::value_type> {
      /// Iterator of the original range.
      Iter it;

      /// Number of elements in the original range to skip when incrementing the iterator.
      std::ptrdiff_t stride{1};

      /// Default constructor.
      stride_iter() = default;

      /**
       * @brief Construct a strided iterator from a given iterator and a given stride.
       * 
       * @param it Iterator of the original range.
       * @param stride Stride for advancing the iterator (has to be > 0).
       */
      stride_iter(Iter it, std::ptrdiff_t stride) : it(it), stride(stride) {
        if (stride <= 0) throw std::runtime_error("The itertools::detail::strided range requires a positive stride");
      }

      /// Increment the iterator by advancing the original iterator by the stride.
      void increment() { std::advance(it, stride); }

      /**
       * @brief Equal-to operator for two detail::stride_iter objects.
       * 
       * @param other detail::stride_iter to compare with.
       * @return True, if the original iterators are equal.
       */
      [[nodiscard]] bool operator==(stride_iter const &other) const { return it == other.it; }

      /**
       * @brief Dereference the iterator.
       * @return Dereferenced value of the original iterator.
       */
      [[nodiscard]] decltype(auto) dereference() const { return *it; }
    };

    /**
     * @brief Represents a strided range.
     * 
     * @details See itertools::stride(R &&, std::ptrdiff_t) for more details.
     * 
     * @tparam R Range type.
     */
    template <typename R> struct strided {
      /// Original range.
      R rg;

      /// Number of elements in the original range to skip when incrementing the iterator.
      std::ptrdiff_t stride;

      /// Iterator type of the strided range.
      using iterator = stride_iter<decltype(std::begin(rg))>;

      /// Const iterator type of the strided range.
      using const_iterator = stride_iter<decltype(std::cbegin(rg))>;

      /// Default equal-to operator.
      [[nodiscard]] bool operator==(strided const &) const = default;

      private:
      /**
       * @brief Helper function to calculate the index of the end iterator.
       * @return Index of the end iterator.
       */
      [[nodiscard]] std::ptrdiff_t end_offset() const {
        auto size = distance(std::cbegin(rg), std::cend(rg));
        return (size == 0) ? 0 : ((size - 1) / stride + 1) * stride;
      }

      public:
      /**
       * @brief Beginning of the strided range.
       * @return detail::stride_iter containing the begin iterator of the original range and the stride.
       */
      [[nodiscard]] iterator begin() noexcept { return {std::begin(rg), stride}; }

      /// Const version of begin().
      [[nodiscard]] const_iterator cbegin() const noexcept { return {std::cbegin(rg), stride}; }

      /// Const overload of begin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the strided range.
       * @return detail::stride_iter containing an iterator of the original range end_offset() elements beyond the 
       * beginning and the stride.
       */
      [[nodiscard]] iterator end() noexcept { return {std::next(std::begin(rg), end_offset()), stride}; }

      /// Const version of end().
      [[nodiscard]] const_iterator cend() const noexcept { return {std::next(std::cbegin(rg), end_offset()), stride}; }

      /// Const overload of end().
      [[nodiscard]] const_iterator end() const noexcept { return cend(); }
    };

  } // namespace detail

  /**
   * @brief Lazy-stride through a given range.
   *
   * @details Only every Nth element of the original range is taken into account. If the given stride (N) is <= 0, an
   * exception is thrown. This function returns an iterable lazy object, which can be used in range-based for loops:
   * 
   * @code{.cpp}
   * std::vector<int> vec { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
   * 
   * for (auto i : stride(vec, 3)) {
   *   std::cout << i << " ";
   * }
   * std::cout << "\n";
   * 
   * for (auto i : stride(vec, 10)) {
   *   std::cout << i << " ";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * 1 4 7 10
   * 1
   * ```
   * 
   * See also See also <a href="https://en.cppreference.com/w/cpp/ranges/stride_view">std::ranges::views::stride</a>.
   *
   * @tparam R Range type.
   * @param rg Original range.
   * @param stride Number of elements to skip when incrementing.
   * @return A detail::strided range.
   */
  template <typename R> [[nodiscard]] detail::strided<R> stride(R &&rg, std::ptrdiff_t stride) { return {std::forward<R>(rg), stride}; }

} // namespace itertools

#endif // _ITERTOOLS_STRIDE_HPP