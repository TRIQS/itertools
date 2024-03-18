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
 * @brief Provides an integer range similar to Python's range.
 */

#ifndef _ITERTOOLS_RANGE_HPP
#define _ITERTOOLS_RANGE_HPP

#include "./product.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace itertools {

  /**
   * @brief A lazy range of integers that mimics a Python range.
   * 
   * @details It stores the first value, the last value (excluded) and the step size between two indices.
   * By default, the step size is set to 1. This function returns an iterable lazy object, which can be 
   * used in range-based for loops:
   * 
   * @code{.cpp}
   * for (auto i : range(5)) {
   *   std::cout << i << " ";
   * }
   * std::cout << "\n";

   * for (auto i : range(-2, 1)) {
   *   std::cout << i << " ";
   * }
   * std::cout << "\n";

   * for (auto i : range(10, 3, -2)) {
   *   std::cout << i << " ";
   * }
   * std::cout << "\n";

   * for (auto i : range(0, 10, -1)) {
   *   std::cout << i << " "; // empty
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * 0 1 2 3 4 
   * -2 -1 0 
   * 10 8 6 4
   * ```
   * 
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/iota_view">std::ranges::views::iota</a>.
   */
  class range {
    /// First value of the range.
    long first_ = 0;

    /// Last value of the range (excluded).
    long last_ = -1;

    /// Number of integers between two elements of the range.
    long step_ = 1;

    public:
    /**
     * @brief Denote a full range at compile-time.
     * @details Can be used for accessing slices of multi-dimensional arrays.
     */
    struct all_t {};

    /// See range::all_t.
    static inline constexpr all_t all = {};

    /// Integer type for backward compatibility.
    using index_t = long;

    /**
     * @brief Default constructor.
     * @deprecated Use range::range(long, long) or range::range(long, long, long) instead.
     */
    [[deprecated("range default construction deprecated. Use range::all for full range in slicing operation")]] range() = default;

    /**
     * @brief Construct a range with a step size of 1 and a given first and last (excluded) value.
     *
     * @param first First value of the range.
     * @param last Last value of the range (excluded).
     */
    range(long first, long last) noexcept : first_(first), last_(last) {}

    /**
     * @brief Construct a range with a given step size and a given first and last (excluded) value.
     * 
     * @details Throws an exception if the step size is zero.
     *
     * @param first First value of the range.
     * @param last Last value of the range (excluded).
     * @param step Number of integers between two elements of the range.
     */
    range(long first, long last, long step) : first_(first), last_(last), step_(step) {
      if (step_ == 0) throw std::runtime_error("Step-size cannot be zero in construction of integer range");
    }

    /**
     * @brief Construct a range with a step size of 1, a first value set to 0 and a given last value (excluded).
     * @param last Last value of the range (excluded).
     */
    explicit range(long last) : range(0, last, 1) {}

    /// Default equal-to operator.
    [[nodiscard]] bool operator==(range const &) const = default;

    /// Get first value of the range.
    [[nodiscard]] long first() const { return first_; }

    /// Get last value of the range (excluded).
    [[nodiscard]] long last() const { return last_; }

    /// Get step size between two elements of the range.
    [[nodiscard]] long step() const { return step_; }

    /// Get number of elements in the range.
    [[nodiscard]] long size() const { return std::max(0l, (last_ + step_ - (step_ > 0 ? 1 : -1) - first_) / step_); }

    /**
     * @brief Shift the whole range by a given amount.
     * 
     * @details Simply adds the given shift to the first and last value of the range, while keeping 
     * the same step size.
     * 
     * @param shift Amount to shift the range by.
     * @return Shifted range.
     */
    [[nodiscard]] range operator+(long shift) const { return {first_ + shift, last_ + shift, step_}; }

    /**
     * @brief Write the range details to std::ostream.
     * 
     * @param os std::ostream object.
     * @param rg range object.
     * @return Reference to os.
     */
    friend inline std::ostream &operator<<(std::ostream &os, const range &rg) {
      os << "range(" << rg.first() << "," << rg.last() << "," << rg.step() << ")";
      return os;
    }

    /// Const iterator type for itertools::range.
    struct const_iterator {
      /// Current value.
      long pos;

      /// Last value of the range (excluded).
      long last;

      /// Step size.
      long step;

      /// Value type.
      using value_type = long;

      /// Iterator category.
      using iterator_category = std::forward_iterator_tag;

      /// Pointer type.
      using pointer = value_type *;

      /// Difference type.
      using difference_type = std::ptrdiff_t;

      /// Reference type.
      using reference = value_type const &;

      /**
       * @brief Pre-increment operator increments the current value by the step size.
       * @return Reference to the incremented iterator.
       */
      const_iterator &operator++() noexcept {
        pos += step;
        return *this;
      }

      /**
       * @brief Post-increment operator increments the current value by the step size.
       * @return Copy of the iterator before incrementing.
       */
      const_iterator operator++(int) noexcept {
        const_iterator c = *this;
        pos += step;
        return c;
      }

      /**
       * @brief Has the iterator reached the end of the range?
       * @return True, if the current value of the iterator is >= the last value of the range for positive step size or
       * if the current value of the iterator is <= the last value of the range for negative step size.
       */
      [[nodiscard]] bool atEnd() const noexcept { return step > 0 ? pos >= last : pos <= last; }

      /**
       * @brief Equal-to operator for two iterators.
       * 
       * @param other Iterator to compare with.
       * @return True, if the current values of both iterators are equal or both iterators are at the end of the range.
       */
      [[nodiscard]] bool operator==(const_iterator const &other) const noexcept {
        return (other.pos == this->pos) || (other.atEnd() && this->atEnd());
      }

      /**
       * @brief Not-equal-to operator for two iterators.
       * 
       * @param other Iterator to compare with.
       * @return True, if the iterators are not equal.
       */
      [[nodiscard]] bool operator!=(const_iterator const &other) const noexcept { return (!operator==(other)); }

      /**
       * @brief Dereference operator.
       * @return Current value of the iterator.
       */
      [[nodiscard]] long operator*() const noexcept { return pos; }

      /**
       * @brief Member access operator.
       * @return Current value of the iterator.
       */
      [[nodiscard]] long operator->() const noexcept { return operator*(); }
    };

    /**
     * @brief Beginning of the integer range.
     * @return Iterator with its current value set to the first value of the range.
     */
    [[nodiscard]] const_iterator begin() const noexcept { return {first_, last_, step_}; }

    /// Const version of begin().
    [[nodiscard]] const_iterator cbegin() const noexcept { return {first_, last_, step_}; }

    /**
     * @brief End of the range.
     * @return Iterator with its current value set to the excluded last value of the range.
     */
    [[nodiscard]] const_iterator end() const noexcept { return {last_, last_, step_}; }

    /// Const version of end().
    [[nodiscard]] const_iterator cend() const noexcept { return {last_, last_, step_}; }
  };

  /**
   * @brief Create a cartesian product range of integer ranges from given integers.
   * 
   * @details The given integers specify the excluded last values of the individual itertools::range objects. 
   * Each range starts at 0 and has a step size of 1.
   * 
   * @code{.cpp}
   * for (auto [i1, i2] : product_range(2, 3)) {
   *   std::cout << "(" << i1 << ", " << i2 << ")\n";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * (0, 0)
   * (0, 1)
   * (0, 2)
   * (1, 0)
   * (1, 1)
   * (1, 2)
   * ```
   * 
   * @tparam Is Integer types.
   * @param is Last values of the integer ranges (excluded).
   * @return Product (detail::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename... Is, typename EnableIf = std::enable_if_t<(std::is_integral_v<Is> and ...), int>> [[nodiscard]] auto product_range(Is... is) {
    return product(range(is)...);
  }

  namespace detail {

    /**
     * @brief Helper function to create a product range of integer ranges from a tuple or an array.
     * 
     * @tparam T Tuple or array type.
     * @param idxs Tuple or array containing the excluded last values of the itertools::range objects.
     * @return Product (detail::multiplied) range of integer ranges.
     */
    template <typename T, size_t... Is> [[gnu::always_inline]] auto product_range_impl(T const &idxs, std::index_sequence<Is...>) {
      return product_range(std::get<Is>(idxs)...);
    }

  } // namespace detail

  /**
   * @brief Create a cartesian product range of integer ranges from a tuple of integers.
   * 
   * @details The integers in the given tuple specify the excluded last values of the individual itertools::range objects. 
   * Each range starts at 0 and has a step size of 1.
   * 
   * @code{.cpp}
   * for (auto [i1, i2] : product_range(std::make_tuple(2, 3))) {
   *   std::cout << "(" << i1 << ", " << i2 << ")\n";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * (0, 0)
   * (0, 1)
   * (0, 2)
   * (1, 0)
   * (1, 1)
   * (1, 2)
   * ```
   * 
   * @tparam Is Integer types.
   * @param idx_tpl Tuple containing the excluded last values of the integer ranges.
   * @return Product (detail::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename... Is, typename EnableIf = std::enable_if_t<(std::is_integral_v<Is> and ...), int>>
  [[nodiscard]] auto product_range(std::tuple<Is...> const &idx_tpl) {
    return detail::product_range_impl(idx_tpl, std::make_index_sequence<sizeof...(Is)>{});
  }

  /**
   * @brief Create a cartesian product range of integer ranges from an array of integers.
   * 
   * @details The integers in the given array specify the excluded last values of the individual itertools::range objects. 
   * Each range starts at 0 and has a step size of 1.
   * 
   * @code{.cpp}
   * for (auto [i1, i2] : product_range(std::array{2, 3})) {
   *   std::cout << "(" << i1 << ", " << i2 << ")\n";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * (0, 0)
   * (0, 1)
   * (0, 2)
   * (1, 0)
   * (1, 1)
   * (1, 2)
   * ```
   * 
   * @tparam I Integer type.
   * @tparam N Number of elements in the array.
   * @param idx_arr Array containing the excluded last values of the integer ranges.
   * @return Product (detail::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename I, size_t N, typename EnableIf = std::enable_if_t<std::is_integral_v<I>, int>>
  [[nodiscard]] auto product_range(std::array<I, N> const &idx_arr) {
    return detail::product_range_impl(idx_arr, std::make_index_sequence<N>{});
  }

  /**
   * @brief Apply a function to every element of an integer itertools::range.
   * 
   * @code{.cpp}
   * // print out the first 10 squares
   * itertools::foreach(itertools::range(1, 11), [](int i) { 
   *   std::cout << i * i << " "; 
   * });
   * @endcode
   * 
   * Output:
   * 
   * ```
   * 1 4 9 16 25 36 49 64 81 100
   * ```
   *
   * @tparam F Callable type.
   * @param rg itertools::range object.
   * @param f Callable object to be applied to each element.
   */
  template <typename F> void foreach (range const &rg, F && f) {
    auto i = rg.first(), last = rg.last(), step = rg.step();
    for (; i < last; i += step) std::forward<F>(f)(i);
  }

} // namespace itertools

#endif // _ITERTOOLS_RANGE_HPP