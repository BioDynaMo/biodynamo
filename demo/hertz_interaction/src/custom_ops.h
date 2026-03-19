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

#ifndef CUSTOM_OPS_H_
#define CUSTOM_OPS_H_

#include "biodynamo.h"
#include "extended_hertz_force.h"
#include "moving_cell.h"

namespace bdm {

struct TrackPosition : public AgentOperationImpl {
  BDM_OP_HEADER(TrackPosition);

  // GetUid()

  void operator()(Agent* agent) override {
    if (auto* cell = bdm_static_cast<Moving_cell*>(agent)) {
      (*positions_)[cell->GetId()].push_back(cell->GetPosition());
    }
  }

  std::vector<std::vector<Double3>>* positions_ = nullptr;
};

struct TrackForce : public StandaloneOperationImpl {
  BDM_OP_HEADER(TrackForce);

  void operator()() override {
    if (forces_ == nullptr || interaction_force_ == nullptr) {
      return;
    }

    auto* sim = Simulation::GetActive();
    if (sim == nullptr) {
      return;
    }

    auto* rm = sim->GetResourceManager();
    auto* agent1 = rm->GetAgent(agent1_uid_);
    auto* agent2 = rm->GetAgent(agent2_uid_);
    if (agent1 == nullptr || agent2 == nullptr) {
      return;
    }

    forces_->push_back(interaction_force_->Calculate(agent1, agent2));
  }

  std::vector<Double4>* forces_ = nullptr;
  AgentUid agent1_uid_;
  AgentUid agent2_uid_;
  ExtendedHertzForce* interaction_force_ = nullptr;
};

}  // namespace bdm

#endif
