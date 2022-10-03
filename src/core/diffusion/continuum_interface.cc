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
#include "core/param/param.h"
#include "core/scheduler.h"
#include "core/simulation.h"

namespace bdm {

void Continuum::IntegrateTimeAsynchronously(real_t dt) {
  if (time_step_ != std::numeric_limits<real_t>::max()) {
    // Uptdate the total time to simulate
    time_to_simulate_ += dt;
    // Compute the number of time steps to simulate
    auto n_steps = static_cast<int>(std::floor(time_to_simulate_ / time_step_));
    // Treat numerical instabilities. E.g. if time_to_simulate_ = 0.19999999999
    // and time_step_ = 0.1, n_steps = 1, but we want n_steps = 2.
    double left_over = time_to_simulate_ - (n_steps + 1) * time_step_;
    const double absolute_tolerance = 1e-9;
    if (left_over < 0 && left_over > -absolute_tolerance) {
      n_steps++;
    }
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

void Continuum::SetTimeStep(real_t dt) { time_step_ = dt; }

real_t Continuum::GetTimeStep() const {
  if (time_step_ != std::numeric_limits<real_t>::max()) {
    return time_step_;
  } else {
    auto* scheduler = Simulation::GetActive()->GetScheduler();
    auto* op = scheduler->GetOps("continuum")[0];
    return Simulation::GetActive()->GetParam()->simulation_time_step *
           op->frequency_;
  }
}
}  // namespace bdm
