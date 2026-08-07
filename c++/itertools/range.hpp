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
 * @brief Provides an integer range similar to Python's range.
 */

#ifndef _ITERTOOLS_RANGE_HPP
#define _ITERTOOLS_RANGE_HPP

#include "./product.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace itertools {

  /**
   * @addtogroup integer_range
   * @{
   */

  /**
   * @brief A lazy range of integers that mimics a Python range.
   *
   * @details It stores the first value, the last value (excluded) and the step size between two indices. By default,
   * the step size is set to 1.
   *
   * This function returns an iterable lazy object, which can be used in range-based for loops:
   *
   * @include doc_range.cpp
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
    // First value of the range.
    long first_;

    // Last value of the range (excluded).
    long last_;

    // Number of integers between two elements of the range.
    long step_ = 1;

    public:
    /**
     * @ingroup integer_range
     * @brief Denote a full range at compile-time.
     * @details Can be used for accessing slices of multi-dimensional arrays.
     */
    struct all_t {};

    /// See range::all_t.
    static inline constexpr all_t all = {};

    /// Integer type for backward compatibility.
    using index_t = long;

    /**
     * @brief Construct a range with a step size of 1 and a given first and last (excluded) value.
     *
     * @param first First value of the range.
     * @param last Last value of the range (excluded).
     */
    range(std::integral auto first, std::integral auto last) noexcept : first_(first), last_(last) {}

    /**
     * @brief Construct a range with a given step size and a given first and last (excluded) value.
     *
     * @details Throws an exception if the step size is zero.
     *
     * @param first First value of the range.
     * @param last Last value of the range (excluded).
     * @param step Number of integers between two elements of the range.
     */
    range(std::integral auto first, std::integral auto last, std::integral auto step) : first_(first), last_(last), step_(step) {
      if (step_ == 0) throw std::runtime_error("Step-size cannot be zero in construction of integer range");
    }

    /**
     * @brief Construct a range with a step size of 1, a first value set to 0 and a given last value (excluded).
     * @param last Last value of the range (excluded).
     */
    explicit range(std::integral auto last) : range(0, last, 1) {}

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
     * @details Simply adds the given shift to the first and last value of the range, while keeping the same step size.
     *
     * @param shift Amount to shift the range by.
     * @return Shifted range.
     */
    [[nodiscard]] range operator+(long shift) const { return {first_ + shift, last_ + shift, step_}; }

    /**
     * @brief Write the range details to `std::ostream`.
     *
     * @param os `std::ostream` object.
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

      /// Step size.
      long step;

      /// Value type.
      using value_type = long;

      /// Iterator category.
      using iterator_category = std::random_access_iterator_tag;

      /// Pointer type.
      using pointer = value_type *;

      /// Difference type.
      using difference_type = std::ptrdiff_t;

      /// Reference type.
      using reference = value_type const &;

      /**
       * @brief Pre-increment operator increments the current value by the step size.
       * @return Reference to `this` iterator.
       */
      const_iterator &operator++() noexcept {
        pos += step;
        return *this;
      }

      /**
       * @brief Post-increment operator increments the current value by the step size.
       * @return Copy of `this` iterator before incrementing.
       */
      const_iterator operator++(int) noexcept {
        const_iterator tmp = *this;
        pos += step;
        return tmp;
      }

      /**
       * @brief Pre-decrement operator decrements the current value by the step size.
       * @return Reference to `this` iterator.
       */
      const_iterator &operator--() noexcept {
        pos -= step;
        return *this;
      }

      /**
       * @brief Post-decrement operator decrements the current value by the step size.
       * @return Copy of `this` iterator before decrementing.
       */
      const_iterator operator--(int) noexcept {
        const_iterator tmp = *this;
        pos -= step;
        return tmp;
      }

      /**
       * @brief Three-way comparison operator for two iterators.
       *
       * @param rhs Right hand side iterator to compare with.
       * @return If the step size is > 0, it returns the result of a three-way comparison of their current values.
       * Otherwise, it three-way compares their negative values.
       */
      [[nodiscard]] std::strong_ordering operator<=>(const_iterator const &rhs) const noexcept {
        return (step > 0 ? pos <=> rhs.pos : -pos <=> -rhs.pos);
      }

      /**
       * @brief Equal-to operator for two iterators.
       *
       * @param other Iterator to compare with.
       * @return True, if the current values of both iterators are equal.
       */
      [[nodiscard]] bool operator==(const_iterator const &other) const noexcept { return pos == other.pos; }

      /**
       * @brief Dereference operator.
       * @return Current value of `this` iterator.
       */
      [[nodiscard]] long operator*() const noexcept { return pos; }

      /**
       * @brief Member access operator.
       * @return Current value of `this` iterator.
       */
      [[nodiscard]] long operator->() const noexcept { return operator*(); }

      /**
       * @brief Addition assignment operator.
       * @param n Number of steps to add to the current value.
       * @return Reference to `this` iterator with its current value increased by the step size multiplied by \f$ n \f$.
       */
      const_iterator &operator+=(difference_type n) noexcept {
        pos += n * step;
        return *this;
      }

      /**
       * @brief Addition operator for an iterator and an integer.
       * @param n Number of steps to add to the current value.
       * @return Copy of `this` object with its current value increased by the step size multiplied by \f$ n \f$.
       */
      [[nodiscard]] const_iterator operator+(difference_type n) const noexcept { return {.pos = pos + n * step, .step = step}; }

      /**
       * @brief Addition operator for an integer and an iterator.
       * @param n Number of steps to add to the current value of the iterator.
       * @param it Iterator.
       * @return Copy of the given iterator with its current value increased by the step size multiplied by \f$ n \f$.
       */
      [[nodiscard]] friend const_iterator operator+(difference_type n, const_iterator it) noexcept { return it + n; }

      /**
       * @brief Subtraction assignment operator.
       * @param n Number of steps to subtract from the current value.
       * @return Reference to `this` iterator with its current value decreased by the step size multiplied by \f$ n \f$.
       */
      const_iterator &operator-=(difference_type n) noexcept {
        pos -= n * step;
        return *this;
      }

      /**
       * @brief Subtraction operator for an iterator and an integer.
       * @param n Number of steps to subtract from the current value.
       * @return Copy of `this` iterator with its current value decreased by the step size multiplied by \f$ n \f$.
       */
      [[nodiscard]] const_iterator operator-(difference_type n) const noexcept { return {.pos = pos - n * step, .step = step}; }

      /**
       * @brief Get the distance between two iterators.
       * @param rhs Right-hand side iterator.
       * @return Number of steps between the two iterators.
       */
      [[nodiscard]] difference_type operator-(const_iterator const &rhs) const noexcept { return (pos - rhs.pos) / step; }

      /**
       * @brief Subscript operator.
       * @param n Number of steps to add to the current value.
       * @return Current value increased by the step size multiplied by \f$ n \f$.
       */
      [[nodiscard]] value_type operator[](difference_type n) const noexcept { return pos + n * step; }

    }; // end struct const_iterator

    /// Reverse const iterator type for itertools::range.
    using const_reverse_iterator = std::reverse_iterator<range::const_iterator>;

    /**
     * @brief Beginning of the integer range.
     * @return Iterator with its current value set to the first value of the range.
     */
    [[nodiscard]] const_iterator cbegin() const noexcept { return {.pos = first_, .step = step_}; }

    /// The same as cbegin().
    [[nodiscard]] const_iterator begin() const noexcept { return {.pos = first_, .step = step_}; }

    /**
     * @brief Beginning of the integer range in reverse order.
     * @return Reverse iterator initialized with end().
     */
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{end()}; }

    /// The same as crbegin().
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }

    /**
     * @brief End of the range.
     * @return Iterator with its current value set to first() + step() * size().
     */
    [[nodiscard]] const_iterator cend() const noexcept { return {.pos = first_ + step_ * size(), .step = step_}; }

    /// The same as cend().
    [[nodiscard]] const_iterator end() const noexcept { return {.pos = first_ + step_ * size(), .step = step_}; }

    /**
     * @brief End of the range in reverse order.
     * @return Reverse iterator initialized with begin().
     */
    [[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator{begin()}; }

    /// The same as crend().
    [[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  };

  /**
   * @brief Create a cartesian product range of integer ranges from given integers.
   *
   * @details The given integers specify the excluded last values of the individual itertools::range objects. Each range
   * starts at 0 and has a step size of 1.
   *
   * @include doc_product_range.cpp
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
   * @return Product (itertools::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename... Is, typename EnableIf = std::enable_if_t<(std::is_integral_v<Is> and ...), int>> [[nodiscard]] auto product_range(Is... is) {
    return product(range(is)...);
  }

  namespace detail {

    // Helper function to create a product range of integer ranges from a tuple or an array.
    template <typename T, size_t... Is> [[gnu::always_inline]] auto product_range_impl(T const &idxs, std::index_sequence<Is...>) {
      return product_range(std::get<Is>(idxs)...);
    }

  } // namespace detail

  /**
   * @brief Create a cartesian product range of integer ranges from a tuple of integers.
   *
   * @details It simply forwards the integers in the given tuple to itertools::product_range.
   *
   * @tparam Is Integer types.
   * @param idx_tpl Tuple containing the excluded last values of the integer ranges.
   * @return Product (itertools::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename... Is, typename EnableIf = std::enable_if_t<(std::is_integral_v<Is> and ...), int>>
  [[nodiscard]] auto product_range(std::tuple<Is...> const &idx_tpl) {
    return detail::product_range_impl(idx_tpl, std::make_index_sequence<sizeof...(Is)>{});
  }

  /**
   * @brief Create a cartesian product range of integer ranges from an array of integers.
   *
   * @details It simply forwards the integers in the given array to itertools::product_range.
   *
   * @tparam I Integer type.
   * @tparam N Number of elements in the array.
   * @param idx_arr Array containing the excluded last values of the integer ranges.
   * @return Product (itertools::multiplied) range of integer ranges. See itertools::product and itertools::range.
   */
  template <typename I, size_t N, typename EnableIf = std::enable_if_t<std::is_integral_v<I>, int>>
  [[nodiscard]] auto product_range(std::array<I, N> const &idx_arr) {
    return detail::product_range_impl(idx_arr, std::make_index_sequence<N>{});
  }

  /**
   * @brief Apply a function to every element of an integer itertools::range.
   *
   * @include doc_for_each.cpp
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

  /** @} */

} // namespace itertools

#endif // _ITERTOOLS_RANGE_HPP
