// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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
    uint64_t stride = 1 << (d + 1);
    uint64_t delta = 1 << d;

#pragma omp parallel for
    for (uint64_t i = delta - 1; i < n - delta; i += stride) {
      v[i + delta] += v[i];
    }
  }

  // downsweep
  for (uint64_t d = 0; d < logn - 1; ++d) {
    uint64_t stride = 1 << (logn - d - 1);
    uint64_t delta = 1 << (logn - d - 2);

#pragma omp parallel for
    for (uint64_t i = stride - 1; i < n - delta; i += stride) {
      v[i + delta] += v[i];
    }
  }
}

}  // namespace bdm

#endif  // CORE_ALGORITHM_H_
