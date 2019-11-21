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

#include "biodynamo.h"
#include "simulation_objects/my_cell.h"

namespace bdm {

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct SpringForce : public BaseBiologyModule {
 public:
  SpringForce() : BaseBiologyModule(gAllEventIds) {}
  SpringForce(double spring_constant = 1)
      : BaseBiologyModule(gAllEventIds), spring_constant_(spring_constant) {}

  SpringForce(const Event& event, BaseBiologyModule* other,
              uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (SpringForce* gdbm = dynamic_cast<SpringForce*>(other)) {
      spring_constant_ = gdbm->spring_constant_;
    } else {
      Log::Fatal("SpringForce::EventConstructor",
                 "other was not of type SpringForce");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new SpringForce(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override { return new SpringForce(*this); }

  // Displacement calculated in the direction of so2
  Double3 CalculateSpringForceDisplacement(MyCell* so1, SoPointer<MyCell> so2) {
    Double3 pos1 = so1->GetPosition();
    Double3 pos2 = so2->GetPosition();
    Double3 force = (pos1 - pos2) * (-spring_constant_);
    double dt = 1;  // TODO: replace with simulation timestep
    Double3 displacement = force * dt * dt;
    std::cout << displacement << std::endl;
    return displacement;
  }

  void Run(SimObject* so) override {
    if (auto* this_cell = dynamic_cast<MyCell*>(so)) {
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
