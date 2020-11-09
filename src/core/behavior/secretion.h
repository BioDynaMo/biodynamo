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

#ifndef CORE_BEHAVIOR_SECRETION_H_
#define CORE_BEHAVIOR_SECRETION_H_

#include "core/behavior/behavior.h"
#include "core/diffusion_grid.h"
#include "core/agent/cell.h"

namespace bdm {

/// Secrete substance at Agent position
struct Secretion : public Behavior {
  BDM_BEHAVIOR_HEADER(Secretion, Behavior, 1);

  Secretion() {}
  Secretion(DiffusionGrid* dgrid, double quantity = 1) 
      : dgrid_(dgrid),
        quantity_(quantity) {}

  virtual ~Secretion() {}

  void Initialize(NewAgentEvent* event) override {
    Base::Initialize(event);
    auto* other = bdm_static_cast<Secretion*>(event->existing_behavior);
    dgrid_ = other->dgrid_;
    quantity_ = other->quantity_;
  }

  void Run(Agent* agent) override {
    auto& secretion_position = agent->GetPosition();
    dgrid_->IncreaseConcentrationBy(secretion_position, quantity_);
  }

 private:
  DiffusionGrid* dgrid_ = nullptr;
  double quantity_ = 1;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_SECRETION_H_
