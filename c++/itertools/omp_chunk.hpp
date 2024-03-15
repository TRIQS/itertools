// Copyright (c) 2019-2020 Simons Foundation
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
// Authors: Olivier Parcollet, Nils Wentzell

#pragma once

#include <omp.h>

#include <itertools/itertools.hpp>

namespace itertools {

  /**
   * @brief Distribute a range as evenly as possible across all OMP threads.
   *
   * @details See chunk_range(std::ptrdiff_t, std::ptrdiff_t, long, long) and slice(R &&, std::ptrdiff_t, std::ptrdiff_t) for more details.
   *
   * @tparam R Range type.
   * @param rg Range to chunk.
   * @return A detail::sliced range, containing the chunk of the original range that belongs to the current thread.
   */
  template <typename R> auto omp_chunk(R &&rg) {
    auto total_size           = itertools::distance(std::cbegin(rg), std::cend(rg));
    auto [start_idx, end_idx] = chunk_range(0, total_size, omp_get_num_threads(), omp_get_thread_num());
    return itertools::slice(std::forward<R>(rg), start_idx, end_idx);
  }

} // namespace itertools
