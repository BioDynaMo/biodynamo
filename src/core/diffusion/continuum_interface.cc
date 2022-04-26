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

#include "continuum_interface.h"

namespace bdm {

void Continuum::IntegrateTimeAsynchronously(double dt) {
  if (time_step_ != std::numeric_limits<double>::max()) {
    time_to_simulate_ += dt;
    auto n_steps = static_cast<int>(std::floor(time_to_simulate_ / time_step_));
    for (int i = 0; i < n_steps; i++) {
      Step(time_step_);
    }
    simulated_time_ += n_steps * time_step_;
    time_to_simulate_ -= n_steps * time_step_;
  } else {
    Step(dt);
    simulated_time_ += dt;
  }
}

}  // namespace bdm
