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
    // Uptdate the total time to simulate
    time_to_simulate_ += dt;
    // Compute the number of time steps to simulate
    auto n_steps = static_cast<int>(std::floor(time_to_simulate_ / time_step_));
    // Simulate for the appropriate number of time steps
    for (int i = 0; i < n_steps; i++) {
      Step(time_step_);
    }
    // Update the total simulated time
    simulated_time_ += n_steps * time_step_;
    // Keep track of time that has not been simulated yet
    time_to_simulate_ -= n_steps * time_step_;
  } else {
    // If time_step_ is not set, we simply forward the time step to the Step
    // method.
    Step(dt);
    simulated_time_ += dt;
  }
}

void Continuum::SetTimeStep(double dt) { time_step_ = dt; }

double Continuum::GetTimeStep() const { return time_step_; }

}  // namespace bdm
