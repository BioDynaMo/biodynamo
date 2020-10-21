// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_MULTI_SIMULATION_DYNAMIC_LOOP_H_
#define CORE_MULTI_SIMULATION_DYNAMIC_LOOP_H_

#include <algorithm>
#include <functional>
#include <vector>

#include "core/multi_simulation/util.h"

namespace bdm {

// Emulates dynamic nested loops
template <typename Lambda>
inline void DynamicNestedLoop(const std::vector<Container*>& containers,
                              const Lambda& action) {
  // Initialize the slots to hold the iterator value for each depth
  auto depth = containers.size();
  if (depth == 0) {
    return;
  }
  std::vector<int> slots(depth, 0);

  // The depth index
  size_t index = 0;
  while (true) {
    action(slots);

    // Increment iterator over outer-most loop
    slots[0]++;

    // Carry
    while (slots[index] == containers[index]->GetNumElements()) {
      // Overflow, we're done
      if (index == depth - 1) {
        return;
      }

      slots[index++] = 0;
      slots[index]++;
    }

    index = 0;
  }
}

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_DYNAMIC_LOOP_H_
