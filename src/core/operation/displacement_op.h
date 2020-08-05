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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_H_
#define CORE_OPERATION_DISPLACEMENT_OP_H_

#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "core/environment/environment.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/scheduler.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/math.h"
#include "core/util/thread_info.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
class DisplacementOp : public OperationImpl {
 public:
  DisplacementOp() {
    auto* tinfo = ThreadInfo::GetInstance();
    last_iteration_.resize(tinfo->GetMaxThreads(),
                           std::numeric_limits<uint64_t>::max());
    last_time_run_.resize(tinfo->GetMaxThreads(), 0);
    delta_time_.resize(tinfo->GetMaxThreads(), 0);
  }

  virtual ~DisplacementOp() {}

  void operator()(SimObject* sim_object) override {
    auto* sim = Simulation::GetActive();
    auto* scheduler = sim->GetScheduler();
    auto* param = sim->GetParam();

    if (!sim_object->RunDisplacement()) {
      return;
    }

    // Update search radius and delta_time_ at beginning of each iteration, and
    // avoid updating them within an iteration
    auto current_iteration = scheduler->GetSimulatedSteps();
    auto tid = omp_get_thread_num();
    if (last_iteration_[tid] != current_iteration) {
      last_iteration_[tid] = current_iteration;

      auto* grid = sim->GetEnvironment();
      auto search_radius = grid->GetLargestObjectSize();
      squared_radius_ = search_radius * search_radius;
      auto current_time =
          (current_iteration + 1) * param->simulation_time_step_;
      delta_time_[tid] = current_time - last_time_run_[tid];
      last_time_run_[tid] = current_time;
    }

    const auto& displacement =
        sim_object->CalculateDisplacement(squared_radius_, delta_time_[tid]);
    sim_object->ApplyDisplacement(displacement);
    if (param->bound_space_) {
      ApplyBoundingBox(sim_object, param->min_bound_, param->max_bound_);
    }
  }

  void TearDown() override {
    squared_radius_ = 0;
    std::fill(last_time_run_.begin(), last_time_run_.end(), 0);
    std::fill(delta_time_.begin(), delta_time_.end(), 0);
    //
    std::fill(last_iteration_.begin(), last_iteration_.end(), -1);
  }

 private:
  double squared_radius_ = 0;
  std::vector<double> last_time_run_;
  std::vector<double> delta_time_;
  std::vector<uint64_t> last_iteration_;
  static bool registered_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_H_
