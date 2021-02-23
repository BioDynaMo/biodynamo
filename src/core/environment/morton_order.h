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

#ifndef CORE_ENVIRONMENT_MORTON_ORDER_H_
#define CORE_ENVIRONMENT_MORTON_ORDER_H_

#include <array>
#include <vector>
#include "core/container/fixed_size_vector.h"

namespace bdm {

class MortonOrder {
 public:
  void Update(const std::array<uint64_t, 3>& num_boxes_axis);
  uint64_t GetIndex(const std::array<uint64_t, 3>& box_coordinates) const;
  FixedSizeVector<uint64_t, 27> GetIndex(
      const FixedSizeVector<std::array<uint64_t, 3>, 27>& boxes) const;
  // private:
  std::vector<std::pair<uint64_t, uint64_t>> offset_index_;
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_MORTON_ORDER_H_
