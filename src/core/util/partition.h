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

#ifndef CORE_UTIL_PARTITION_H_
#define CORE_UTIL_PARTITION_H_

#include <cmath>

namespace bdm {

inline void Partition(uint64_t elements, uint64_t batches, uint64_t batch_num,
                      uint64_t* start, uint64_t* end) {
  auto correction = elements % batches == 0 ? 0 : 1;
  auto chunk = elements / batches + correction;
  *start = batch_num * chunk;
  *end = std::min(elements, *start + chunk);
}

}  // namespace bdm

#endif  // CORE_UTIL_PARTITION_H_
