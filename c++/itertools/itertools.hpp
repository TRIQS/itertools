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
 * @brief Provides a small subset of the ranges and views from [std::ranges](https://en.cppreference.com/w/cpp/ranges).
 */

#ifndef _ITERTOOLS_HPP
#define _ITERTOOLS_HPP

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace itertools {

  // Forward declaration.
  template <typename Iter, typename Value, typename Tag = std::forward_iterator_tag, typename Reference = Value &,
            typename Difference = std::ptrdiff_t>
  struct iterator_facade;

  /**
   * @brief CRTP base class for various iterator types in itertools.
   * 
   * @details All iterator types defined in itertools are derived from this class. It uses the 
   * <a href="https://en.cppreference.com/w/cpp/iterator/forward_iterator">forward iterator</a>
   * category. Derived classes are required to implement the following member functions:
   * 
   * @code{.cpp}
   * // used by operator++() and operator++(int)
   * void increment();
   * 
   * // used by operator*() and operator->()
   * value_type [const] [&] dereference() [const];
   * @endcode
   * 
   * The `[..]` are optional and depend on the actual iterator type.
   * 
   * @tparam Iter Derived iterator type.
   * @tparam Value Value type of the iterator.
   * @tparam Reference Reference type of the iterator.
   * @tparam Difference Difference type of the iterator.
   */
  template <typename Iter, typename Value, typename Reference, typename Difference>
  struct iterator_facade<Iter, Value, std::forward_iterator_tag, Reference, Difference> {
    private:
    /// Get a reference to the derived iterator.
    [[nodiscard]] Iter &self() { return static_cast<Iter &>(*this); }

    /// Get a const reference to the derived iterator.
    [[nodiscard]] Iter const &self() const { return static_cast<const Iter &>(*this); }

    public:
    /// Value type of the derived iterator.
    using value_type = Value;

    /// Reference type of the derived iterator.
    using reference = Reference;

    /// Pointer type of the derived iterator.
    using pointer = Value *;

    /// Difference type of the derived iterator.
    using difference_type = Difference;

    /// Iterator category of the derived iterator.
    using iterator_category = std::forward_iterator_tag;

    /**
     * @brief Pre-increment operator.
     * @return Reference to the derived iterator after calling the derived iterator's `increment()` function.
     */
    Iter &operator++() {
      self().increment();
      return self();
    }

    /**
     * @brief Post-increment operator.
     * @return Copy of the derived iterator before calling the derived iterator's `increment()` function.
     */
    Iter operator++(int) {
      Iter c = self();
      self().increment();
      return c;
    }

    /**
     * @brief Dereference operator.
     * @return Result of the derived iterator's `dereference()` function.
     */
    [[nodiscard]] decltype(auto) operator*() const { return self().dereference(); }

    /**
     * @brief Member access operator.
     * @return Result of the derived iterator's `dereference()` function.
     */
    [[nodiscard]] decltype(auto) operator->() const { return operator*(); }
  };

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
   * @brief Generic sentinel type that can be used to mark the end of a range.
   * @tparam Iter Iterator type.
   */
  template <typename Iter> struct sentinel_t {
    /// End iterator of some range.
    Iter it;
  };

  /**
   * @brief Create an itertools::sentinel_t from an iterator using template type deduction.
   * 
   * @tparam Iter Iterator type.
   * @param it Iterator to be turned into an itertools::sentinel_t.
   * @return Sentinel object.
   */
  template <typename Iter> [[nodiscard]] sentinel_t<Iter> make_sentinel(Iter it) { return {std::move(it)}; }

  namespace detail {

    /**
     * @brief Iterator for a detail::enumerated range.
     * 
     * @details It stores an iterator of the original range and an index. Incrementing advances the iterator
     * and the index by 1. Dereferencing returns a std::pair consisting of the current index and the current 
     * dereferenced value of the original iterator.
     * 
     * See itertools::enumerate(R &&) for more details.
     * 
     * @tparam Iter Iterator type.
     */
    template <typename Iter> struct enum_iter : iterator_facade<enum_iter<Iter>, std::pair<long, typename std::iterator_traits<Iter>::value_type>> {
      /// Iterator of the original range.
      Iter it;

      /// Index for enumerating.
      long i = 0;

      /// Default constructor sets the index to zero and default constructs the original iterator.
      enum_iter() = default;

      /**
       * @brief Construct an enumerated iterator from a given iterator and set the index to zero.
       * @param it Iterator of the original range.
       */
      enum_iter(Iter it) : it(std::move(it)) {}

      /// Increment the iterator by incrementing the original iterator and the index.
      void increment() {
        ++it;
        ++i;
      }

      /**
       * @brief Equal-to operator for two detail::enum_iter objects.
       * 
       * @param other detail::enum_iter to compare with.
       * @return True, if the original iterators are equal.
       */
      [[nodiscard]] bool operator==(enum_iter const &other) const { return it == other.it; }

      /**
       * @brief Equal-to operator for a detail::enum_iter and an itertools::sentinel_t.
       * 
       * @tparam SentinelIter Iterator type of the sentinel.
       * @param s itertools::sentinel_t to compare with.
       * @return True, if the original iterator is equal to the iterator stored in the sentinel.
       */
      template <typename SentinelIter> [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const &s) const { return it == s.it; }

      /**
       * @brief Dereference the iterator.
       * @return Tuple consisting of the current index and the current dereferenced value of the original iterator.
       */
      [[nodiscard]] decltype(auto) dereference() const { return std::tuple<long, decltype(*it)>{i, *it}; }
    };

    /**
     * @brief Iterator for a detail::transformed range.
     * 
     * @details It stores an iterator of the original range and a callable object that is used to transform the
     * elements of the original range. Incrementing simply increments the iterator. Dereferencing returns the 
     * result of the callable object applied to the dereferenced iterator, i.e. the transformed element. 
     * 
     * See itertools::transform(R &&, F) for more details.
     * 
     * @tparam Iter Iterator type.
     * @tparam F Callable type.
     * @tparam Value Return type of the callable.
     */
    template <typename Iter, typename F, typename Value = std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type>>
    struct transform_iter : iterator_facade<transform_iter<Iter, F>, Value> {
      /// Iterator of the original range.
      Iter it;

      /// Callable doing the transformation.
      mutable std::optional<F> lambda;

      /// Default constructor.
      transform_iter() = default;

      /**
       * @brief Construct a transformed iterator from a given iterator and callable.
       * 
       * @param it Iterator of the original range.
       * @param lambda Callable doing the transformation.
       */
      transform_iter(Iter it, F lambda) : it(std::move(it)), lambda(std::move(lambda)) {}

      /// Increment the iterator by incrementing the original iterator.
      void increment() { ++it; }

      /// Default move constructor.
      transform_iter(transform_iter &&) = default;

      /// Default copy constructor.
      transform_iter(transform_iter const &) = default;

      /// Default move assignment operator.
      transform_iter &operator=(transform_iter &&) = default;

      /**
       * @brief Custom copy assignment operator makes sure that the optional callable is correctly copied.
       * @param other detail::transform_iter to copy from.
       */
      transform_iter &operator=(transform_iter const &other) {
        it = other.it;
        if (other.lambda.has_value())
          lambda.emplace(other.lambda.value());
        else
          lambda.reset();
        return *this;
      }

      /**
       * @brief Equal-to operator for two detail::transform_iter objects.
       * 
       * @param other detail::transform_iter to compare with.
       * @return True, if the original iterators are equal.
       */
      [[nodiscard]] bool operator==(transform_iter const &other) const { return it == other.it; }

      /**
       * @brief Equal-to operator for a detail::transform_iter and an itertools::sentinel_t.
       * 
       * @tparam SentinelIter Iterator type of the sentinel.
       * @param s itertools::sentinel_t to compare with.
       * @return True, if the original iterator is equal to the iterator stored in the sentinel.
       */
      template <typename SentinelIter> [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const &s) const { return (it == s.it); }

      /**
       * @brief Dereference the iterator.
       * @return Result of the callable applied to the dereferenced iterator, i.e. the transformed value.
       */
      [[nodiscard]] decltype(auto) dereference() const { return (*lambda)(*it); }
    };

    /**
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
     * @brief Iterator for a detail::multiplied (cartesian product) range.
     * 
     * @details It stores three tuples of iterators of the original ranges:
     * - `its_begin` contains the begin iterators of all ranges
     * - `its_end` contains the end iterators of all ranges
     * - `its` contains the current iterators of all ranges
     * 
     * Incrementing increments one iterator at a time in row-major order, i.e. if one iterator is equal to its
     * end iterator, it is reset to its begin iterator and the previous iterator is incremented. Dereferencing 
     * returns a tuple containing the results of dereferencing each iterator.
     * 
     * See itertools::product(Rs &&...rgs) for more details.
     * 
     * @tparam EndIters Tuple type containing the end iterators of all ranges.
     * @tparam Iters Iterator types.
     */
    template <typename EndIters, typename... Iters>
    struct prod_iter : iterator_facade<prod_iter<EndIters, Iters...>, std::tuple<typename std::iterator_traits<Iters>::value_type...>> {
      /// Tuple containing the begin iterators of the original ranges.
      std::tuple<Iters...> its_begin;

      /// Tuple containing the end iterators of the original ranges.
      EndIters its_end;

      /// Tuple containing the current iterators of the original ranges.
      std::tuple<Iters...> its = its_begin;

      /// Number of original ranges.
      static constexpr long Rank = sizeof...(Iters);

      /// Default constructor.
      prod_iter() = default;

      /**
       * @brief Construct a product iterator from given begin iterators and end iterators.
       * 
       * @param its_begin Tuple containing begin iterators of the original ranges.
       * @param its_end Tuple containing end iterators of the original ranges.
       */
      prod_iter(std::tuple<Iters...> its_begin, EndIters its_end) : its_begin(std::move(its_begin)), its_end(std::move(its_end)) {}

      private:
      /**
       * @brief Helper function which recursively increments the current iterators in row-major order.
       * @tparam N Iterator index to increment.
       */
      template <int N> void _increment() {
        // increment Nth iterator
        ++std::get<N>(its);
        // recursively increment previous iterators if necessary
        if constexpr (N > 0) {
          // if Nth iterator is at its end, reset it to its begin iterator and increment N-1st iterator
          if (std::get<N>(its) == std::get<N>(its_end)) {
            std::get<N>(its) = std::get<N>(its_begin);
            _increment<N - 1>();
          }
        }
      }

      public:
      /// Increment the iterator by incrementing the current iterators in row-major order.
      void increment() { _increment<Rank - 1>(); }

      /**
       * @brief Equal-to operator for two detail::prod_iter objects.
       * 
       * @param other detail::prod_iter to compare with.
       * @return True, if all original iterators are equal.
       */
      [[nodiscard]] bool operator==(prod_iter const &other) const { return its == other.its; }

      /**
       * @brief Equal-to operator for a detail::prod_iter and an itertools::sentinel_t.
       * 
       * @details Since we traverse the cartesian product in row-major order, we reach the end of the product range,
       * when the first iterator, i.e. `std::get<0>(its)`, is at its end.
       * 
       * @tparam SentinelIter Iterator type of the sentinel.
       * @param s itertools::sentinel_t to compare with.
       * @return True, if the first iterator, i.e. `std::get<0>(its)`, is equal to the iterator of the sentinel.
       */
      template <typename SentinelIter> [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const &s) const { return (s.it == std::get<0>(its)); }

      private:
      /**
       * @brief Helper function to dereference all original iterators.
       * @return Tuple containing the dereferenced values of all original iterators.
       */
      template <size_t... Is> [[gnu::always_inline]] [[nodiscard]] auto tuple_map_impl(std::index_sequence<Is...>) const {
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
     * @brief Represents a transformed range.
     * 
     * @details See itertools::transform(R &&, F) for more details.
     * 
     * @tparam R Range type.
     * @tparam F Callable type.
     */
    template <typename R, typename F> struct transformed {
      /// Original range.
      R rg;

      /// Callable doing the transformation.
      F lambda;

      /// Const iterator type of the transformed range.
      using const_iterator = transform_iter<decltype(std::cbegin(rg)), F>;

      /// Iterator type of the transformed range.
      using iterator = const_iterator;

      /**
       * @brief Beginning of the transformed range.
       * @return detail::transform_iter constructed from the beginning of the original range and the callable.
       */
      [[nodiscard]] const_iterator cbegin() const noexcept { return {std::cbegin(rg), lambda}; }

      /// Non-const version of cbegin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the transformed range.
       * @return itertools::sentinel_t containing the end iterator of the original range.
       */
      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(rg)); }

      /// Non-const version of cend().
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    /**
     * @brief Represents an enumerated range.
     * 
     * @details See itertools::enumerate(R &&) for more details.
     * 
     * @tparam R Range type.
     */
    template <typename R> struct enumerated {
      /// Original range.
      R rg;

      /// Iterator type of the enumerated range.
      using iterator = enum_iter<decltype(std::begin(rg))>;

      /// Const iterator type of the enumerated range.
      using const_iterator = enum_iter<decltype(std::cbegin(rg))>;

      /// Default equal-to operator.
      [[nodiscard]] bool operator==(enumerated const &) const = default;

      /**
       * @brief Beginning of the enumerated range.
       * @return detail::enum_iter constructed from the beginning of the original range with the index set to zero.
       */
      [[nodiscard]] iterator begin() noexcept { return std::begin(rg); }

      /// Const version of begin().
      [[nodiscard]] const_iterator cbegin() const noexcept { return std::cbegin(rg); }

      /// Const overload of begin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the enumerated range.
       * @return itertools::sentinel_t containing the end iterator of the original range.
       */
      [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(rg)); }

      /// Const version of end().
      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(rg)); }

      /// Const overload of end().
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    /**
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

    /**
     * @brief Represents a cartesian product of ranges.
     * 
     * @details See itertools::product(Rs &&...rgs) for more details.
     * 
     * @tparam Rs Range types.
     */
    template <typename... Rs> struct multiplied {
      /// Tuple containing the original ranges.
      std::tuple<Rs...> tu;

      /// Iterator type of the product range.
      using iterator = prod_iter<std::tuple<decltype(std::end(std::declval<Rs &>()))...>, decltype(std::begin(std::declval<Rs &>()))...>;

      /// Const iterator type the product range.
      using const_iterator = prod_iter<std::tuple<decltype(std::cend(std::declval<Rs &>()))...>, decltype(std::cbegin(std::declval<Rs &>()))...>;

      /**
       * @brief Constructs a cartesian product (multiplied) range from the given ranges.
       * 
       * @tparam Us Range types.
       * @param rgs Ranges to be multiplied.
       */
      template <typename... Us> multiplied(Us &&...rgs) : tu{std::forward<Us>(rgs)...} {}

      /// Default equal-to operator.
      [[nodiscard]] bool operator==(multiplied const &) const = default;

      private:
      /// Helper function to create a detail::prod_iter representing the beginning of the product range.
      template <size_t... Is> [[gnu::always_inline]] auto _begin(std::index_sequence<Is...>) {
        return iterator{std::make_tuple(std::begin(std::get<Is>(tu))...), std::make_tuple(std::end(std::get<Is>(tu))...)};
      }

      /// Const version of _begin(std::index_sequence<Is...>).
      template <size_t... Is> [[gnu::always_inline]] auto _cbegin(std::index_sequence<Is...>) const {
        return const_iterator{std::make_tuple(std::cbegin(std::get<Is>(tu))...), std::make_tuple(std::cend(std::get<Is>(tu))...)};
      }

      public:
      /**
       * @brief Beginning of the product range.
       * @return detail::prod_iter representing the beginning of the product range.
       */
      [[nodiscard]] iterator begin() noexcept { return _begin(std::index_sequence_for<Rs...>{}); }

      /// Const version of begin().
      [[nodiscard]] const_iterator cbegin() const noexcept { return _cbegin(std::index_sequence_for<Rs...>{}); }

      /// Const overload of begin().
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      /**
       * @brief End of the product range.
       * @return itertools::sentinel_t containing the end iterator of the first original range, i.e. `std::end(std::get<0>(tu))`.
       */
      [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(std::get<0>(tu))); }

      /// Const version of end().
      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(std::get<0>(tu))); }

      /// Const overload of end().
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    // Class template argument deduction guide.
    template <typename... T> multiplied(T &&...) -> multiplied<std::decay_t<T>...>;

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
   * @brief Lazy-transform a given range by applying a unary callable object to every element of the original range. 
   * 
   * @details The value type of the transformed range depends on the return type of the callable.
   * This function returns an iterable lazy object (a detail::transformed range), which can be used in range-based for loops:
   * 
   * @code{.cpp}
   * std::list<int> list { 1, 2, 3, 4, 5 };
   * 
   * for (auto i : itertools::transform(list, [](int i) { return i * i; })) {
   *   std::cout << i << " ";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * 1 4 9 16 25
   * ```
   *  
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/transform_view">std::ranges::views::transform</a>.
   * 
   * @tparam R Range type.
   * @tparam F Callable type.
   * @param rg Range to transform.
   * @param lambda Callable to be applied to the given range.
   * @return A detail::transformed range.
   */
  template <typename R, typename F> [[nodiscard]] auto transform(R &&rg, F lambda) {
    return detail::transformed<R, F>{std::forward<R>(rg), std::move(lambda)};
  }

  /**
   * @brief Lazy-enumerate a given range (similar to Python's enumerate).
   * 
   * @details Each element in the original range is assigned an index, starting from zero. This function 
   * returns an iterable lazy object (a detail::enumerated range), which iterates over tuples consisting of the
   * index and the value of the dereferenced iterator of the original range:
   * 
   * @code{.cpp}
   * std::vector<char> vec { 'a', 'b', 'c' };
   * 
   * for (auto [idx, val] : enumerate(vec)) {
   *   std::cout << "(" << idx << ", " << val << ")\n";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * (0, a)
   * (1, b)
   * (2, c)
   * ```
   * 
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/enumerate_view">std::ranges::views::enumerate</a>.
   *
   * @tparam R Range type.
   * @param rg Range to enumerate.
   * @return A detail::enumerated range.
   */
  template <typename R> [[nodiscard]] detail::enumerated<R> enumerate(R &&rg) { return {std::forward<R>(rg)}; }

  /**
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

  /**
   * @brief Combine zip and transform.
   *
   * @details The given ranges are first zipped and then transformed with the given callable object. The callable
   * object is called with the dereferenced iterators of the zipped range, i.e. if N ranges are zipped it needs to
   * be callable with N arguments. This function returns an iterable lazy object, which can be used in range-based
   * for loops:
   * 
   * @code{.cpp}
   * for (auto x : zip_with(lambda, rg1, rg2, rg3)) {
   *   // do something with x
   * }
   * @endcode
   * 
   * Note that the type of x depends on the return type of the `lambda` function.
   * 
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/zip_transform_view">std::ranges::views::zip_transform</a>.
   *
   * @tparam Rs Range types.
   * @tparam F Callable type.
   * @param rgs Ranges to zip.
   * @param f Callable to be applied to the zipped range.
   * @return A zipped range.
   */
  template <typename... Rs, typename F> auto zip_with(Rs &&...rgs, F &&f) {
    return transform(zip(std::forward<Rs>(rgs)...), [&f](std::tuple<Rs...> t) { return std::apply(std::forward<F>(f), t); });
  }

  /**
   * @brief Lazy-multiply a given number of ranges by forming their cartesian product.
   *
   * @details An arbitrary number of ranges are multiplied together into a cartesian product range. They are traversed in a row-major
   * order, i.e. the last range is the fastest dimension. The number of elements in a product range is equal to the product of 
   * the sizes of the given ranges. This function returns an iterable lazy object, which can be used in range-based for loops:
   * 
   * @code{.cpp}
   * std::vector<int> v1 { 1, 2, 3 };
   * std::vector<char> v2 { 'a', 'b' };
   * 
   * for (auto [i, c] : product(v1, v2)) {
   *   std::cout << "(" << i << ", " << c << ")\n";
   * }
   * @endcode
   * 
   * Output:
   * 
   * ```
   * (1, a)
   * (1, b)
   * (2, a)
   * (2, b)
   * (3, a)
   * (3, b)
   * ```
   * 
   * See also <a href="https://en.cppreference.com/w/cpp/ranges/cartesian_product_view">std::ranges::views::cartesian_product</a>.
   *
   * @tparam Rs Range types.
   * @param rgs Ranges to be used.
   * @return A product (detail::multiplied) range.
   */
  template <typename... Rs> [[nodiscard]] detail::multiplied<Rs...> product(Rs &&...rgs) { return {std::forward<Rs>(rgs)...}; }

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

  namespace detail {

    /**
     * @brief Helper function to create a product range from a container of ranges.
     * 
     * @tparam C Container type.
     * @param cont Container of ranges.
     * @return Product range from the ranges in the container.
     */
    template <typename C, size_t... Is> [[gnu::always_inline]] [[nodiscard]] auto make_product_impl(C &cont, std::index_sequence<Is...>) {
      return product(cont[Is]...);
    }

  } // namespace detail

  /**
   * @brief Create a cartesian product range from an array of ranges.
   * 
   * @tparam R Range type.
   * @tparam N Number of ranges.
   * @param arr Array of ranges.
   * @return A product (detail::multiplied) range from the ranges in the array.
   */
  template <typename R, size_t N> [[nodiscard]] auto make_product(std::array<R, N> &arr) {
    return detail::make_product_impl(arr, std::make_index_sequence<N>{});
  }

  /// Const overload of make_product(std::array<R, N> &).
  template <typename R, size_t N> [[nodiscard]] auto make_product(std::array<R, N> const &arr) {
    return detail::make_product_impl(arr, std::make_index_sequence<N>{});
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

#endif // _ITERTOOLS_HPP_
