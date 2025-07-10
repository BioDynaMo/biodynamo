// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_ALGORITHM_H_
#define CORE_ALGORITHM_H_

#include <cmath>
#include <cstdint>

namespace bdm {

// -----------------------------------------------------------------------------
/// Calculate work-efficient inclusive prefix sum.
/// Calculation is parallel and in-place.
template <typename T>
void InPlaceParallelPrefixSum(T& v, uint64_t n) {
  if (n < 2) {
    return;
  }

  // upsweep
  uint64_t logn = static_cast<uint64_t>(std::ceil(std::log2(n)));
  for (uint64_t d = 0; d < logn; ++d) {
    uint64_t stride = 1ULL << (d + 1);
    uint64_t delta = 1ULL << d;

#pragma omp parallel for
    for (uint64_t i = delta - 1; i < n - delta; i += stride) {
      v[i + delta] += v[i];
    }
  }

  // downsweep
  if (logn >= 2) {  // Only execute if logn >= 2 to ensure no underflow
    for (uint64_t d = 0; d < logn - 1; ++d) {
      // At this point: d < logn - 1, so d <= logn - 2
      // Therefore: logn >= d + 2
      // This guarantees: logn - d - 1 >= 1 and logn - d - 2 >= 0
      
      uint64_t shift_amount1 = logn - d - 1;
      uint64_t shift_amount2 = logn - d - 2;
      
      // Prevent undefined behavior from large shift amounts (>= 64)
      if (shift_amount1 >= 64 || shift_amount2 >= 64) {
        break;
      }
      
      uint64_t stride = 1ULL << shift_amount1;
      uint64_t delta = 1ULL << shift_amount2;

#pragma omp parallel for
      for (uint64_t i = stride - 1; i < n - delta; i += stride) {
        v[i + delta] += v[i];
      }
    }
  }
}

// -----------------------------------------------------------------------------
/// Calculate exclusive prefix sum in-place.
/// n must be <= t->size() - 1
/// This means that there must be an additional element in the vector at v[n+1]
template <typename T>
void ExclusivePrefixSum(T* v, uint64_t n) {
  auto tmp = (*v)[0];
  (*v)[0] = 0;
  for (uint64_t i = 1; i <= n; ++i) {
    auto result = (*v)[i - 1] + tmp;
    tmp = (*v)[i];
    (*v)[i] = result;
  }
}

// -----------------------------------------------------------------------------
// if search_val is found in container, return right-most occurrence.
// If not return the index of the right-most element that is smaller.
// If no smaller element exists, return element at index 0
template <typename TSearch, typename TContainer>
uint64_t BinarySearch(const TSearch& search_val, const TContainer& container,
                      uint64_t from, uint64_t to) {
  if (to <= from) {
    if (container[from] != search_val && from > 0) {
      // if (from < container.size() && container[from] != search_val && from >
      // 0) {
      from--;
    }
    return from;
  }

  auto m = (from + to) / 2;
  if (container[m] == search_val) {
    if (m + 1 <= to && container[m + 1] == search_val) {
      return BinarySearch(search_val, container, m + 1, to);
    }
    return m;
  } else if (container[m] > search_val) {
    return BinarySearch(search_val, container, from, m);
  } else {
    return BinarySearch(search_val, container, m + 1, to);
  }
}

}  // namespace bdm

#endif  // CORE_ALGORITHM_H_
