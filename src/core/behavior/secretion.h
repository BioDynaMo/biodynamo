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

#ifndef CORE_BEHAVIOR_SECRETION_H_
#define CORE_BEHAVIOR_SECRETION_H_

#include <string>

#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/resource_manager.h"

namespace bdm {

/// Secrete substance at Agent position
class Secretion : public Behavior {
  BDM_BEHAVIOR_HEADER(Secretion, Behavior, 2);

 public:
  Secretion() = default;
  explicit Secretion(const std::string& substance, real_t quantity = 1,
                     InteractionMode mode = InteractionMode::kAdditive)
      : quantity_(quantity), mode_(mode) {
    dgrid_ = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(
        substance);
  }

  explicit Secretion(DiffusionGrid* dgrid, real_t quantity = 1,
                     InteractionMode mode = InteractionMode::kAdditive)
      : dgrid_(dgrid), quantity_(quantity), mode_(mode) {}

  virtual ~Secretion() = default;

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = bdm_static_cast<Secretion*>(event.existing_behavior);
    dgrid_ = other->dgrid_;
    quantity_ = other->quantity_;
  }

  void Run(Agent* agent) override {
    auto& secretion_position = agent->GetPosition();
    dgrid_->ChangeConcentrationBy(secretion_position, quantity_, mode_);
  }

 private:
  DiffusionGrid* dgrid_ = nullptr;
  real_t quantity_ = 1;
  InteractionMode mode_ = InteractionMode::kAdditive;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_SECRETION_H_
