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
  Secretion() {}
  explicit Secretion(std::string substance, double quantity = 1)
      : substance_(substance), quantity_(quantity) {
    dgrid_ = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(
        substance);
  }

  virtual ~Secretion() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = bdm_static_cast<Secretion*>(event.existing_behavior);
    substance_ = other->substance_;
    dgrid_ = other->dgrid_;
    quantity_ = other->quantity_;
  }

  void Run(Agent* agent) override {
    auto& secretion_position = agent->GetPosition();
    dgrid_->ChangeConcentrationBy(secretion_position, quantity_);
  }

 private:
  std::string substance_ = 0;
  DiffusionGrid* dgrid_ = nullptr;
  double quantity_ = 1;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_SECRETION_H_
