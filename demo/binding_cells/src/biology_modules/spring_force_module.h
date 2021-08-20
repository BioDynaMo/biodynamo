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
#ifndef SPRING_FORCE_MODULE_H_
#define SPRING_FORCE_MODULE_H_

#include "agents/monocyte.h"
#include "agents/t_cell.h"
#include "core/behavior/behavior.h"

namespace bdm {

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct SpringForce : public Behavior {
  BDM_BEHAVIOR_HEADER(SpringForce, Behavior, 1);

 public:
  SpringForce(double spring_constant = 1) : spring_constant_(spring_constant) {
    AlwaysCopyToNew();
  }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = event.existing_behavior;
    if (SpringForce* gdbm = dynamic_cast<SpringForce*>(other)) {
      spring_constant_ = gdbm->spring_constant_;
    } else {
      Log::Fatal("SpringForce::EventConstructor",
                 "other was not of type SpringForce");
    }
  }

  // Displacement calculated in the direction of ap2
  Double3 CalculateSpringForceDisplacement(TCell* ap1,
                                           AgentPointer<Monocyte> ap2) {
    Double3 pos1 = ap1->GetPosition();
    Double3 pos2 = ap2->GetPosition();
    Double3 force = (pos1 - pos2) * (-spring_constant_);
    auto dt = Simulation::GetActive()->GetParam()->simulation_time_step;
    Double3 displacement = force * dt * dt;
    return displacement;
  }

  void Run(Agent* agent) override {
    if (auto* this_cell = dynamic_cast<TCell*>(agent)) {
      if (this_cell->IsConnected()) {
        auto other_cell = this_cell->GetConnectedCell();
        auto displacement =
            CalculateSpringForceDisplacement(this_cell, other_cell);
        this_cell->UpdatePosition(displacement);
      }
    }
  }

 private:
  double spring_constant_;
};

}  // namespace bdm

#endif  // SPRING_FORCE_MODULE_H_
