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

#ifndef CORE_BEHAVIOR_CHEMOTAXIS_H_
#define CORE_BEHAVIOR_CHEMOTAXIS_H_

#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/resource_manager.h"

namespace bdm {

/// Move cells along the diffusion gradient (from low concentration to high)
class Chemotaxis : public Behavior {
  BDM_BEHAVIOR_HEADER(Chemotaxis, Behavior, 1);

 public:
  Chemotaxis() {}
  Chemotaxis(std::string substance, double speed)
      : substance_(substance), speed_(speed) {
    dgrid_ = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(
        substance);
  }

  explicit Chemotaxis(DiffusionGrid* dgrid, double speed)
      : dgrid_(dgrid), speed_(speed) {
    substance_ = dgrid->GetSubstanceName();
  }

  virtual ~Chemotaxis() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = bdm_static_cast<Chemotaxis*>(event.existing_behavior);
    substance_ = other->substance_;
    dgrid_ = other->dgrid_;
    speed_ = other->speed_;
  }

  void Run(Agent* agent) override {
    auto* cell = bdm_static_cast<Cell*>(agent);
    auto& position = cell->GetPosition();
    Double3 gradient;
    dgrid_->GetGradient(position, &gradient);  // returns normalized gradient
    cell->UpdatePosition(gradient * speed_);
  }

 private:
  std::string substance_;
  DiffusionGrid* dgrid_ = nullptr;
  double speed_;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_CHEMOTAXIS_H_
