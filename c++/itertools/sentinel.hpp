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
 * @brief Provides a generic sentinel type for various iterator types in itertools.
 */

#ifndef _ITERTOOLS_SENTINEL_HPP
#define _ITERTOOLS_SENTINEL_HPP

#include <utility>

namespace itertools {

  /**
   * @addtogroup utilities
   * @{
   */

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

  /** @} */

} // namespace itertools

#endif // _ITERTOOLS_SENTINEL_HPP