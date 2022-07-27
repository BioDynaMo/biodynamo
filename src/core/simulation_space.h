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
#ifndef CORE_SIMULATION_SPACE_H_
#define CORE_SIMULATION_SPACE_H_

#include "core/container/math_array.h"

namespace bdm {

struct SimulationSpace {
  virtual ~SimulationSpace() {}
  MathArray<double, 6> whole_space;
};

// FIXME move to separate file
struct DistributedSimSpace : public SimulationSpace {
  virtual ~DistributedSimSpace() {}
  MathArray<double, 6> local_space;
};

}  // namespace bdm

#endif  // CORE_SIMULATION_SPACE_H_
