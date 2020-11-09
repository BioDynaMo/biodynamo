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
#include "core/agent/agent.h"
#include "core/simulation.h"
#include "core/util/math.h"
#include "core/util/thread_info.h"
#include "core/interaction_force.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
struct DisplacementOp : public AgentOperationImpl {
  BDM_OP_HEADER(DisplacementOp);

  DisplacementOp() : force_(new InteractionForce()) {
    auto* tinfo = ThreadInfo::GetInstance();
    last_iteration_.resize(tinfo->GetMaxThreads(),
                           std::numeric_limits<uint64_t>::max());
    last_time_run_.resize(tinfo->GetMaxThreads(), 0);
    delta_time_.resize(tinfo->GetMaxThreads(), 0);
  }

  DisplacementOp(const DisplacementOp& other) 
    : squared_radius_(other.squared_radius_),
      last_time_run_(other.last_time_run_),
      delta_time_(other.delta_time_),
      last_iteration_(other.last_iteration_)
  {
    if (other.force_) {
      force_ = other.force_->NewCopy();
    } 
  }

  virtual ~DisplacementOp() {
    if (force_) {
      delete force_;
    }
  }

  void SetInteractionForce(InteractionForce* force) {
    if (force == force_) {
      return;
    }
    if (force_) {
      delete force_;
    }
    force_ = force;
  }

  void operator()(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* scheduler = sim->GetScheduler();
    auto* param = sim->GetParam();

    if (!agent->RunDisplacement()) {
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
          (current_iteration + 1) * param->simulation_time_step;
      delta_time_[tid] = current_time - last_time_run_[tid];
      last_time_run_[tid] = current_time;
    }

    const auto& displacement =
        agent->CalculateDisplacement(force_, squared_radius_, delta_time_[tid]);
    agent->ApplyDisplacement(displacement);
    if (param->bound_space) {
      ApplyBoundingBox(agent, param->min_bound, param->max_bound);
    }
  }

 private:
  InteractionForce* force_ = nullptr;
  double squared_radius_ = 0;
  std::vector<double> last_time_run_;
  std::vector<double> delta_time_;
  std::vector<uint64_t> last_iteration_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_H_
