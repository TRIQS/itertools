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

#ifndef _ITERTOOLS_HPP
#define _ITERTOOLS_HPP

#include <tuple>
#include <vector>
#include <iterator>
#include <iostream>
#include <exception>
#include <optional>

namespace itertools {

  template <class Iter, class Value, class Tag = std::forward_iterator_tag, class Reference = Value &, class Difference = std::ptrdiff_t>
  struct iterator_facade;

  /*
   * A helper for the implementation of forward iterators using CRTP
   *
   * @tparam Iter
   * The Iterator Class to be implemented
   * `Iter` is required to have the following member functions
   * - value_type [const] [&] dereference()
   * - void increment()
   */
  template <typename Iter, typename Value, typename Reference, typename Difference>
  struct iterator_facade<Iter, Value, std::forward_iterator_tag, Reference, Difference> {

    private:
    Iter &self() { return static_cast<Iter &>(*this); }
    [[nodiscard]] Iter const &self() const { return static_cast<const Iter &>(*this); }

    public:
    using value_type        = Value;
    using reference         = Reference;
    using pointer           = Value *;
    using difference_type   = Difference;
    using iterator_category = std::forward_iterator_tag;

    Iter &operator++() {
      self().increment();
      return self();
    }

    Iter operator++(int) {
      Iter c = self();
      self().increment();
      return c;
    }

    decltype(auto) operator*() const { return self().dereference(); }
    decltype(auto) operator->() const { return operator*(); }
  };

  template <typename Iter, typename EndIter> inline typename std::iterator_traits<Iter>::difference_type distance(Iter first, EndIter last) {
    if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
      // Difference should be defined also for the case that last is a sentinel
      return last - first;
    } else {
      typename std::iterator_traits<Iter>::difference_type r(0);
      for (; first != last; ++first) ++r;
      return r;
    }
  }

  // Sentinel_t, used to denote the end of certain ranges
  template <typename It> struct sentinel_t {
    It it;
  };
  template <typename It> sentinel_t<It> make_sentinel(It it) { return {std::move(it)}; }

  namespace detail {

    /********************* Enumerate Iterator ********************/

    template <typename Iter> struct enum_iter : iterator_facade<enum_iter<Iter>, std::pair<long, typename std::iterator_traits<Iter>::value_type>> {

      Iter it;
      long i = 0;

      enum_iter() = default;
      enum_iter(Iter it) : it(std::move(it)) {}

      void increment() {
        ++it;
        ++i;
      }

      bool operator==(enum_iter const &other) const { return it == other.it; }

      template <typename OtherSentinel> bool operator==(sentinel_t<OtherSentinel> const &other) const { return it == other.it; }

      [[nodiscard]] decltype(auto) dereference() const { return std::tuple<long, decltype(*it)>{i, *it}; }
    };

    /********************* Transform Iterator ********************/

    template <typename Iter, typename L, typename Value = std::invoke_result_t<L, typename std::iterator_traits<Iter>::value_type>>
    struct transform_iter : iterator_facade<transform_iter<Iter, L>, Value> {

      Iter it;
      mutable std::optional<L> lambda;

      transform_iter() = default;
      transform_iter(Iter it, L lambda) : it(std::move(it)), lambda(std::move(lambda)) {}

      void increment() { ++it; }

      transform_iter(transform_iter &&)                 = default;
      transform_iter(transform_iter const &)            = default;
      transform_iter &operator=(transform_iter &&other) = default;

      transform_iter &operator=(transform_iter const &other) {
        it = other.it;
        if (other.lambda.has_value())
          lambda.emplace(other.lambda.value());
        else
          lambda.reset();
        return *this;
      }

      bool operator==(transform_iter const &other) const { return it == other.it; }

      template <typename OtherSentinel> bool operator==(sentinel_t<OtherSentinel> const &other) const { return (it == other.it); }

      decltype(auto) dereference() const { return (*lambda)(*it); }
    };

    /********************* Zip Iterator ********************/

    template <typename... It> struct zip_iter : iterator_facade<zip_iter<It...>, std::tuple<typename std::iterator_traits<It>::value_type...>> {

      std::tuple<It...> its;

      zip_iter() = default;
      zip_iter(std::tuple<It...> its) : its(std::move(its)) {}

      private:
      template <size_t... Is> [[gnu::always_inline]] void increment_all(std::index_sequence<Is...>) { ((void)(++std::get<Is>(its)), ...); }

      public:
      void increment() { increment_all(std::index_sequence_for<It...>{}); }

      bool operator==(zip_iter const &other) const { return its == other.its; }

      template <typename OtherSentinel> bool operator==(sentinel_t<OtherSentinel> const &other) const {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return ((std::get<Is>(its) == std::get<Is>(other.it)) || ...);
        }(std::index_sequence_for<It...>{});
      }

      template <size_t... Is> [[nodiscard]] auto tuple_map_impl(std::index_sequence<Is...>) const {
        return std::tuple<decltype(*std::get<Is>(its))...>(*std::get<Is>(its)...);
      }

      [[nodiscard]] decltype(auto) dereference() const { return tuple_map_impl(std::index_sequence_for<It...>{}); }
    };

    /********************* Product Iterator ********************/

    template <typename TupleSentinel, typename... It>
    struct prod_iter : iterator_facade<prod_iter<TupleSentinel, It...>, std::tuple<typename std::iterator_traits<It>::value_type...>> {

      std::tuple<It...> its_begin;
      TupleSentinel its_end;
      std::tuple<It...> its      = its_begin;
      static constexpr long Rank = sizeof...(It);

      prod_iter() = default;
      prod_iter(std::tuple<It...> its_begin, TupleSentinel its_end) : its_begin(std::move(its_begin)), its_end(std::move(its_end)) {}

      template <int N> void _increment() {
        ++std::get<N>(its);
        if constexpr (N > 0) {
          if (std::get<N>(its) == std::get<N>(its_end)) {
            std::get<N>(its) = std::get<N>(its_begin);
            _increment<N - 1>();
          }
        }
      }
      void increment() { _increment<Rank - 1>(); }

      bool operator==(prod_iter const &other) const { return its == other.its; }

      template <typename U> bool operator==(sentinel_t<U> const &s) const { return (s.it == std::get<0>(its)); }

      private:
      template <size_t... Is> [[gnu::always_inline]] [[nodiscard]] auto tuple_map_impl(std::index_sequence<Is...>) const {
        return std::tuple<decltype(*std::get<Is>(its))...>(*std::get<Is>(its)...);
      }

      public:
      [[nodiscard]] decltype(auto) dereference() const { return tuple_map_impl(std::index_sequence_for<It...>{}); }
    };

    /********************* Stride Iterator ********************/

    template <typename Iter> struct stride_iter : iterator_facade<stride_iter<Iter>, typename std::iterator_traits<Iter>::value_type> {

      Iter it;
      std::ptrdiff_t stride;

      stride_iter() = default;
      stride_iter(Iter it, std::ptrdiff_t stride) : it(it), stride(stride) {
        if (stride <= 0) throw std::runtime_error("strided range requires a positive stride");
      }

      void increment() { std::advance(it, stride); }

      bool operator==(stride_iter const &other) const { return it == other.it; }

      decltype(auto) dereference() const { return *it; }
    };

    /********************* The Wrapper Classes representing the adapted ranges ********************/

    template <typename T, typename L> struct transformed {
      T x;
      L lambda;

      using const_iterator = transform_iter<decltype(std::cbegin(x)), L>;
      using iterator       = const_iterator;

      [[nodiscard]] const_iterator cbegin() const noexcept { return {std::cbegin(x), lambda}; }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(x)); }
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    // ---------------------------------------------

    template <typename T> struct enumerated {
      T x;

      using iterator       = enum_iter<decltype(std::begin(x))>;
      using const_iterator = enum_iter<decltype(std::cbegin(x))>;

      bool operator==(enumerated const &) const = default;

      [[nodiscard]] iterator begin() noexcept { return std::begin(x); }
      [[nodiscard]] const_iterator cbegin() const noexcept { return std::cbegin(x); }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(x)); }
      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(x)); }
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    // ---------------------------------------------

    template <typename... T> struct zipped {
      std::tuple<T...> tu; // T can be a ref.

      using seq_t          = std::index_sequence_for<T...>;
      using iterator       = zip_iter<decltype(std::begin(std::declval<T &>()))...>;
      using const_iterator = zip_iter<decltype(std::cbegin(std::declval<T &>()))...>;

      template <typename... U> zipped(U &&...ranges) : tu{std::forward<U>(ranges)...} {}

      bool operator==(zipped const &) const = default;

      private:
      // Apply function to tuple
      template <typename F, size_t... Is> [[gnu::always_inline]] auto tuple_map(F &&f, std::index_sequence<Is...>) {
        return std::make_tuple(f(std::get<Is>(tu))...);
      }
      template <typename F, size_t... Is> [[gnu::always_inline]] auto tuple_map(F &&f, std::index_sequence<Is...>) const {
        return std::make_tuple(f(std::get<Is>(tu))...);
      }

      public:
      [[nodiscard]] iterator begin() noexcept {
        return tuple_map([](auto &&x) { return std::begin(x); }, seq_t{});
      }
      [[nodiscard]] const_iterator cbegin() const noexcept {
        return tuple_map([](auto &&x) { return std::cbegin(x); }, seq_t{});
      }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] auto end() noexcept {
        return make_sentinel(tuple_map([](auto &&x) { return std::end(x); }, seq_t{}));
      }
      [[nodiscard]] auto cend() const noexcept {
        return make_sentinel(tuple_map([](auto &&x) { return std::cend(x); }, seq_t{}));
      }
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    // ---------------------------------------------

    template <typename... T> struct multiplied {
      std::tuple<T...> tu; // T can be a ref.

      using iterator       = prod_iter<std::tuple<decltype(std::end(std::declval<T &>()))...>, decltype(std::begin(std::declval<T &>()))...>;
      using const_iterator = prod_iter<std::tuple<decltype(std::cend(std::declval<T &>()))...>, decltype(std::cbegin(std::declval<T &>()))...>;

      template <typename... U> multiplied(U &&...ranges) : tu{std::forward<U>(ranges)...} {}

      bool operator==(multiplied const &) const = default;

      private:
      template <size_t... Is> [[gnu::always_inline]] auto _begin(std::index_sequence<Is...>) {
        return iterator{std::make_tuple(std::begin(std::get<Is>(tu))...), std::make_tuple(std::end(std::get<Is>(tu))...)};
      }

      template <size_t... Is> [[gnu::always_inline]] auto _cbegin(std::index_sequence<Is...>) const {
        return const_iterator{std::make_tuple(std::cbegin(std::get<Is>(tu))...), std::make_tuple(std::cend(std::get<Is>(tu))...)};
      }

      public:
      [[nodiscard]] iterator begin() noexcept { return _begin(std::index_sequence_for<T...>{}); }
      [[nodiscard]] const_iterator cbegin() const noexcept { return _cbegin(std::index_sequence_for<T...>{}); }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(std::get<0>(tu))); }
      [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(std::get<0>(tu))); }
      [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    template <typename... T> multiplied(T &&...) -> multiplied<std::decay_t<T>...>;

    // ---------------------------------------------

    template <typename T> struct sliced {
      T x;
      std::ptrdiff_t start_idx, end_idx;

      using iterator       = decltype(std::begin(x));
      using const_iterator = decltype(std::cbegin(x));

      bool operator==(sliced const &) const = default;

      [[nodiscard]] std::ptrdiff_t size() const {
        std::ptrdiff_t total_size = distance(std::cbegin(x), std::cend(x));
        return std::min(total_size, end_idx) - start_idx;
      }

      [[nodiscard]] iterator begin() noexcept { return std::next(std::begin(x), start_idx); }
      [[nodiscard]] const_iterator cbegin() const noexcept { return std::next(std::cbegin(x), start_idx); }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] iterator end() noexcept { return std::next(begin(), size()); }
      [[nodiscard]] const_iterator cend() const noexcept { return std::next(cbegin(), size()); }
      [[nodiscard]] const_iterator end() const noexcept { return cend(); }
    };

    // ---------------------------------------------

    template <typename T> struct strided {
      T x;
      std::ptrdiff_t stride;

      using iterator       = stride_iter<decltype(std::begin(x))>;
      using const_iterator = stride_iter<decltype(std::cbegin(x))>;

      bool operator==(strided const &) const = default;

      [[nodiscard]] iterator begin() noexcept { return {std::begin(x), stride}; }
      [[nodiscard]] const_iterator cbegin() const noexcept { return {std::cbegin(x), stride}; }
      [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

      [[nodiscard]] iterator end() noexcept {
        std::ptrdiff_t end_idx = distance(std::cbegin(x), std::cend(x));
        return std::next(std::end(x), stride - end_idx % stride);
      }
      [[nodiscard]] const_iterator cend() const noexcept {
        std::ptrdiff_t end_idx = distance(std::cbegin(x), std::cend(x));
        return std::next(std::cend(x), stride - end_idx % stride);
      }
      [[nodiscard]] const_iterator end() const noexcept { return cend(); }
    };

  } // namespace detail

  /********************* The range adapting functions ********************/

  /**
   * Transform (lazy)applies a unary lambda function to every
   * element of a range. It returns itself a range.
   *
   * @param range The range that the lambda is applied to
   * @param range The lambda to apply to the range
   */
  template <typename T, typename L> auto transform(T &&range, L lambda) {
    return detail::transformed<T, L>{std::forward<T>(range), std::move(lambda)};
  }

  /**
   * Lazy-enumerate a range (similar to Python enumerate)
   * 
   * The function returns a iterable lazy object. When iterated upon, 
   * this object yields a pair (n,x) where :
   *   * n is the index (starting at 0)
   *   * x is in the object in the range
   *
   * @tparam R Type of the ranges
   * @param range The range to enumerate
   * @example itertools/enumerate.cpp
   */
  template <typename R> detail::enumerated<R> enumerate(R &&range) { return {std::forward<R>(range)}; }

  /**
   * Generate a zip of the ranges (similar to Python zip).
   *
   * The function returns a iterable lazy object. When iterated upon, 
   * this object yields a tuple of the objects in the ranges. 
   *
   * @tparam R Type of the ranges
   * @param ranges 
   *     The ranges to zip. 
   *
   *     .. warning::
   *          The ranges have to be equal lengths or behaviour is undefined.
   */
  template <typename... R> detail::zipped<R...> zip(R &&...ranges) { return {std::forward<R>(ranges)...}; }

  /**
   * Generate a zip of the ranges (similar to Python zip).
   *
   *  DOC TO BE WRITTEN.
   *
   * @param ranges 
   * @param lambda 
   * @tparam R Type of the ranges
   * @tparam L Type of the Lambda
   */
  template <typename... T, typename L> auto zip_with(T &&...ranges, L &&lambda) {
    return transform(zip(std::forward<T>(ranges)...), [lambda](std::tuple<T...> t) { return std::apply(lambda, t); });
  }

  /**
   * Lazy-product of multiple ranges. This function returns itself a range of tuple<T...>.
   * Iterating over it will yield all combinations of the different range values.
   * Note: The ranges are incremented beginning with the leftmost range.
   * Note: The length is equal to the minimum of the lengths of all ranges.
   *
   * @tparam T The types of the different ranges
   * @param ranges The ranges to zip.
   */
  template <typename... T> detail::multiplied<T...> product(T &&...ranges) { return {std::forward<T>(ranges)...}; }

  /**
   * Lazy-slice a range.
   * This function returns itself a slice of the initial range
   *
   * @param range The range to slice
   * @param start_idx The index to start the slice at
   * @param end_idx The index one past the end of the sliced range
   */
  template <typename T> detail::sliced<T> slice(T &&range, std::ptrdiff_t start_idx, std::ptrdiff_t end_idx) {
    return {std::forward<T>(range), start_idx, std::max(start_idx, end_idx)};
  }

  /**
   * Lazy-stride a range.
   * This function returns itself a subrange of the initial range
   * by considering only every N-th element
   *
   * @param range The range to take the subrange of
   * @param stride The numer of elements to skip
   */
  template <typename T> detail::strided<T> stride(T &&range, std::ptrdiff_t stride) { return {std::forward<T>(range), stride}; }

  /********************* Some factory functions ********************/

  namespace detail {
    template <typename A, size_t... Is> [[gnu::always_inline]] auto make_product_impl(A &arr, std::index_sequence<Is...>) {
      return product(arr[Is]...);
    }
  } // namespace detail

  template <typename T, size_t N> auto make_product(std::array<T, N> &arr) { return detail::make_product_impl(arr, std::make_index_sequence<N>{}); }

  template <typename T, size_t N> auto make_product(std::array<T, N> const &arr) {
    return detail::make_product_impl(arr, std::make_index_sequence<N>{});
  }

  template <typename R> auto make_vector_from_range(R const &r) {
    std::vector<std::decay_t<decltype(*(std::begin(r)))>> vec{}; // decltype returns a &
    if constexpr (std::is_same_v<decltype(std::cbegin(r)), decltype(std::cend(r))>) {
      auto total_size = distance(std::cbegin(r), std::cend(r));
      vec.reserve(total_size);
    }
    for (auto const &x : r) vec.emplace_back(x);
    return vec;
  }

  /********************* Functionality related to Integer ranges ********************/

  /**
   * A range of integer indices that mimics the Python `range`.
   */
  class range {
    long first_ = 0, last_ = -1, step_ = 1;

    public:
    // Denote the full range at compile-time by variable all of a seperate type
    struct all_t {};
    static inline constexpr all_t all = {};

    // Keep alias index_t for backward compatibility
    using index_t = long;

    /**
     * Default constructor - Deprecated
     *
     * Note: For full index range in slicing use range::all
     * */
    [[deprecated("range default construction deprecated. Use range::all for full range in slicing operation")]] range() = default;

    /**
     * Constructor
     *
     * @param first: First index of the range
     * @param last: End of the range (excluded)
     *
     * @examples :
     *
     *      A(range (0,3), 0)  // means  A(0,0), A(1,0), A(2,0)
     *      A(range (0,4,2), 0) // means A(0,0), A(2,0)  
     * */
    range(long first, long last) noexcept : first_(first), last_(last) {}

    /**
     * Constructor
     *
     * @param first: First index of the range
     * @param last: End of the range (excluded)
     * @param step: Step-size between two indices
     *
     * @examples :
     *
     *      A(range (0,3), 0)  // means  A(0,0), A(1,0), A(2,0)
     *      A(range (0,4,2), 0) // means A(0,0), A(2,0)  
     * */
    range(long first, long last, long step) : first_(first), last_(last), step_(step) {
      if (step_ == 0) throw std::runtime_error("Step-size cannot be zero in construction of integer range");
    }

    /**
     * Constructor
     *
     * @param last: End of the range (excluded)
     *
     * Equivalent to range(0,last,1)
     */
    explicit range(long last) : range(0, last, 1) {}

    bool operator==(range const &) const = default;

    /// First index of the range
    [[nodiscard]] long first() const { return first_; }

    /// End of the range (excluded)
    [[nodiscard]] long last() const { return last_; }

    /// Step-size between two indices
    [[nodiscard]] long step() const { return step_; }

    /// Number of indices in the range
    [[nodiscard]] long size() const { return std::max(0l, (last_ + step_ - (step_ > 0 ? 1 : -1) - first_) / step_); }

    range operator+(long shift) const { return {first_ + shift, last_ + shift, step_}; }

    friend inline std::ostream &operator<<(std::ostream &os, const range &r) {
      os << "range(" << r.first() << "," << r.last() << "," << r.step() << ")";
      return os;
    }

    // Iterator on the range (for for loop e.g.)
    class const_iterator {

      public:
      long pos, last, step;

      using value_type        = long;
      using iterator_category = std::forward_iterator_tag;
      using pointer           = value_type *;
      using difference_type   = std::ptrdiff_t;
      using reference         = value_type const &;

      const_iterator &operator++() noexcept {
        pos += step;
        return *this;
      }

      const_iterator operator++(int) noexcept {
        const_iterator c = *this;
        pos += step;
        return c;
      }

      [[nodiscard]] bool atEnd() const noexcept { return step > 0 ? pos >= last : pos <= last; }

      bool operator==(const_iterator const &other) const noexcept {
        // EXPECTS(other.last == this->last);
        // EXPECTS(other.step == this->step);
        return (other.pos == this->pos) || (other.atEnd() && this->atEnd());
      }
      bool operator!=(const_iterator const &other) const noexcept { return (!operator==(other)); }

      long operator*() const noexcept { return pos; }
      long operator->() const noexcept { return operator*(); }
    };

    [[nodiscard]] const_iterator begin() const noexcept { return {first_, last_, step_}; }
    [[nodiscard]] const_iterator cbegin() const noexcept { return {first_, last_, step_}; }

    [[nodiscard]] const_iterator end() const noexcept { return {last_, last_, step_}; }
    [[nodiscard]] const_iterator cend() const noexcept { return {last_, last_, step_}; }
  };

  /**
   * A product of an arbitrary number of integer ranges
   * given a set of integers or an integer tuple
   *
   * @tparam Integers The integer types
   */
  template <typename... Integers, typename EnableIf = std::enable_if_t<(std::is_integral_v<Integers> and ...), int>>
  auto product_range(Integers... Is) {
    return product(range(Is)...);
  }

  namespace detail {
    template <typename Tuple, size_t... Is> [[gnu::always_inline]] auto product_range_impl(Tuple const &idx_tpl, std::index_sequence<Is...>) {
      return product_range(std::get<Is>(idx_tpl)...);
    }
  } // namespace detail

  template <typename... Integers, typename EnableIf = std::enable_if_t<(std::is_integral_v<Integers> and ...), int>>
  auto product_range(std::tuple<Integers...> const &idx_tpl) {
    return detail::product_range_impl(idx_tpl, std::make_index_sequence<sizeof...(Integers)>{});
  }

  template <typename Integer, size_t Rank, typename EnableIf = std::enable_if_t<std::is_integral_v<Integer>, int>>
  auto product_range(std::array<Integer, Rank> const &idx_arr) {
    return detail::product_range_impl(idx_arr, std::make_index_sequence<Rank>{});
  }

  /**
   * Given an integer range [start, end), chunk it as equally as possible into n_chunks.
   * If the range is not dividable in n_chunks equal parts, the first chunks have
   * one more element than the last ones.
   */
  inline std::pair<std::ptrdiff_t, std::ptrdiff_t> chunk_range(std::ptrdiff_t start, std::ptrdiff_t end, long n_chunks, long rank) {
    auto total_size    = end - start;
    auto chunk_size    = total_size / n_chunks;
    auto n_large_nodes = total_size - n_chunks * chunk_size;
    if (rank < n_large_nodes) // larger nodes have size chunk_size + 1
      return {start + rank * (chunk_size + 1), start + (rank + 1) * (chunk_size + 1)};
    else // smaller nodes have size chunk_size
      return {start + n_large_nodes + rank * chunk_size, start + n_large_nodes + (rank + 1) * chunk_size};
  }

  /**
   * Apply a function f to every element of an integer range
   *
   * @param r
   * The range to apply the function to
   *
   * @param f
   * The function to apply
   */
  template <typename F> void foreach (range const &r, F && f) {
    auto i = r.first(), last = r.last(), step = r.step();
    for (; i < last; i += step) f(i);
  }

} // namespace itertools

#endif
