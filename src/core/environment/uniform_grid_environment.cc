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

#include "core/environment/uniform_grid_environment.h"

namespace bdm {

using NeighborMutex = Environment::NeighborMutexBuilder::NeighborMutex;
using GridNeighborMutexBuilder =
    UniformGridEnvironment::GridNeighborMutexBuilder;

NeighborMutex* GridNeighborMutexBuilder::GetMutex(uint64_t box_idx) {
  auto* grid = static_cast<UniformGridEnvironment*>(
      Simulation::GetActive()->GetEnvironment());
  FixedSizeVector<uint64_t, 27> box_indices;
  grid->GetMooreBoxIndices(&box_indices, box_idx);
  thread_local GridNeighborMutex* mutex =
      new GridNeighborMutex(box_indices, this);
  mutex->SetMutexIndices(box_indices);
  return mutex;
}

}  // namespace bdm
