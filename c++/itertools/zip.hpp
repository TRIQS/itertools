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
 * @brief Provides a range adapting function for zipping a given number of ranges/views.
 */

#ifndef _ITERTOOLS_ZIP_HPP
#define _ITERTOOLS_ZIP_HPP

#include "./iterator_facade.hpp"
#include "./sentinel.hpp"

#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>

namespace itertools {

  namespace detail {

    /**
     * @ingroup range_iterators
     * @brief Iterator for a detail::zipped range.
     *
     * @details It stores iterators of the original ranges in a tuple. Incrementing simply increments each iterator
     * individually. Dereferencing returns a tuple containing the results of dereferencing each iterator.
     *
     * See itertools::zip(Rs &&...rgs) for more details.
     *
     * @tparam Iters Iterator types.
     */
    template <typename... Iters>
    struct zip_iter : iterator_facade<zip_iter<Iters...>, std::tuple<typename std::iterator_traits<Iters>::value_type...>> {
      /// Tuple containing iterators of the original ranges.
      std::tuple<Iters...> its;

      /// Default constructor.
      zip_iter() = default;

      /**
       * @brief Constructs a zipped iterator from given iterators.
       * @param its Tuple containing iterators of the original ranges.
       */
      zip_iter(std::tuple<Iters...> its) : its(std::move(its)) {}

      private:
      /// Helper function which increments all original iterators.
      template <size_t... Is> [[gnu::always_inline]] void increment_all(std::index_sequence<Is...>) { ((void)(++std::get<Is>(its)), ...); }

      public:
      /// Increment the iterator by incrementing all original iterators stored in the tuple.
      void increment() { increment_all(std::index_sequence_for<Iters...>{}); }

      /**
       * @brief Equal-to operator for two detail::zip_iter objects.
       *
       * @param other detail::zip_iter to compare with.
       * @return True, if all original iterators are equal.
       */
      [[nodiscard]] bool operator==(zip_iter const &other) const { return its == other.its; }

      /**
       * @brief Equal-to operator for a detail::zip_iter and an itertools::sentinel_t.
       *
       * @details Only one of the iterators has to be equal to the corresponding iterator of the sentinel. In case
       * the original ranges have different lengths, the detail::zipped range should have the length of the shortest range.
       *
       * @tparam SentinelIter Iterator type of the sentinel.
       * @param s itertools::sentinel_t to compare with.
       * @return True, if one of the original iterators is equal to the corresponding iterator of the sentinel.
       */
      template <typename SentinelIter> [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const &s) const {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return ((std::get<Is>(its) == std::get<Is>(s.it)) || ...);
        }(std::index_sequence_for<Iters...>{});
      }

      private:
      /**
       * @brief Helper function to dereference all original iterators.
       * @return Tuple containing the dereferenced values of all original iterators.
       */
      template <size_t... Is> [[nodiscard]] auto tuple_map_impl(std::index_sequence<Is...>) const {
        return std::tuple<decltype(*std::get<Is>(its))...>(*std::get<Is>(its)...);
      }

      public:
      /**
       * @brief Dereference the iterator.
       * @return Tuple containing the dereferenced values of all original iterators.
       */
      [[nodiscard]] decltype(auto) dereference() const { return tuple_map_impl(std::index_sequence_for<Iters...>{}); }
    };

    /**
     * @ingroup adapted_ranges
     * @brief Represents a zipped range.
     *
     * @details See itertools::zip(Rs &&...rgs) for more details.
     *
     * @tparam Rs Range types.
     */
    template <typename... Rs> struct zipped {
      /// Tuple containing the original ranges.
      std::tuple<Rs...> tu;

      /// Convenience typedef for an std::index_sequence.
      using seq_t = std::index_sequence_for<Rs...>;

      /// Iterator type of the zipped range.
      using iterator = zip_iter<decltype(std::begin(std::declval<Rs &>()))...>;

      /// Const iterator type of the zipped range.
      using const_iterator = zip_iter<decltype(std::cbegin(std::declval<Rs &>()))...>;

      /**
       * @brief Construct a zipped range from the given ranges.
       *
       * @tparam Us Range types.
       * @param rgs Ranges to zip.
       */
      template <typename... Us> zipped(Us &&...rgs) : tu{std::forward<Us>(rgs)...} {}

      /// Default equal-to operator.
      [[nodiscard]] bool operator==(zipped const &) const = default;

      private:
      /**
       * @brief Helper function that applies a given callable to each range in the stored tuple and returns a
       * new tuple with the results.
       *
       * @tparam F Callable type.
       * @param f Callable object.
       * @return Tuple containing the mapped tuple elements after applying the callable.
       */
      template <typename F, size_t... Is> [[gnu::always_inline]] auto tuple_map(F &&f, std::index_sequence<Is...>) {
        return std::make_tuple(std::forward<F>(f)(std::get<Is>(tu))...);
      }

      /// Const overload of tuple_map(F &&, std::index_sequence<Is...>).
      template <typename F, size_t... Is> [[gnu::always_inline]] auto tuple_map(F &&f, std::index_sequence<Is...>) const {
        return std::make_tuple(std::forward<F>(f)(std::get<Is>(tu))...);
      }

      public:
      /**
       * @brief Beginning of the zipped range.
       * @return detail::zip_iter constructed from the begin iterators of the original ranges.
       */
      [[nodiscard]] iterator begin() noexcept {
        return tuple_map([](auto &&rg) { return std::begin(rg); }, seq_t{});
      }

      /// Const version of begin().
      [[nodiscard]] const_iterator cbegin() const noexcept {
        return tuple_map([](auto &&rg) { return std::cbegin(rg); }, seq_t{});
      }

      /// Const overload of begin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the zipped range.
       * @return itertools::sentinel_t containing a tuple of end iterators of the original ranges.
       */
      [[nodiscard]] auto end() noexcept {
        return make_sentinel(tuple_map([](auto &&rg) { return std::end(rg); }, seq_t{}));
      }

      /// Const version of end().
      [[nodiscard]] auto cend() const noexcept {
        return make_sentinel(tuple_map([](auto &&rg) { return std::cend(rg); }, seq_t{}));
      }

      /// Const overload of end().
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

  } // namespace detail

  /**
   * @ingroup range_adapting_functions
   * @brief Lazy-zip ranges together (similar to Python's zip).
   *
   * @details An arbitrary number of ranges are zipped together into a tuple. The zipped range will have as many
   * elements as the shortest given range. This function returns an iterable lazy object, which can be used in
   * range-based for loops:
   *
   * @code{.cpp}
   * std::vector<int> v1 { 1, 2, 3 };
   * std::vector<char> v2 { 'a', 'b', 'c', 'd', 'e' };
   *
   * for (auto [i1, i2] : zip(v1, v1)) {
   *   std::cout << "(" << i1 << ", " << i2 << ") ";
   * }
   *
   * for (auto [i1, i2, c3] : zip(v1, v1, v2)) {
   *   std::cout << "(" << i1 << ", " << i2 << ", " << c3 << ") ";
   * }
   * @endcode
   *
   * Output:
   *
   * ```
   * (1, 1) (2, 2) (3, 3)
   * (1, 1, a) (2, 2, b) (3, 3, c)
   * ```
   *
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/zip_view">std::ranges::views::zip</a>.
   *
   * @tparam Rs Range types.
   * @param rgs Ranges to zip.
   * @return A detail::zipped range.
   */
  template <typename... Rs> [[nodiscard]] detail::zipped<Rs...> zip(Rs &&...rgs) { return {std::forward<Rs>(rgs)...}; }

} // namespace itertools

#endif // _ITERTOOLS_ZIP_HPP