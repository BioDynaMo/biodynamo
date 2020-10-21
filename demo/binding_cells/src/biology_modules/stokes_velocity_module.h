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
#ifndef STOKES_VELOCITY_MODULE_H_
#define STOKES_VELOCITY_MODULE_H_

#include "biodynamo.h"
#include "simulation_objects/t_cell.h"

namespace bdm {

/// Modeling of Stokes' law of terminal velocity of sphere falling in a fluid
/// Source: https://en.wikipedia.org/wiki/Stokes%27_law
struct StokesVelocity : public BaseBiologyModule {
 public:
  StokesVelocity() : BaseBiologyModule(gAllEventIds) {}

  StokesVelocity(double u, double pf)
      : BaseBiologyModule(gAllEventIds), u_(u), pf_(pf) {
    if (std::abs(u) < 1e-9) {
      Log::Fatal("StokesVelocity::Run()",
                 "u_ was found to be (very close to) zero!");
    }
  }

  StokesVelocity(const Event& event, BaseBiologyModule* other,
                 uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (StokesVelocity* gdbm = dynamic_cast<StokesVelocity*>(other)) {
      u_ = gdbm->u_;
      pf_ = gdbm->pf_;
    } else {
      Log::Fatal("StokesVelocity::EventConstructor",
                 "other was not of type StokesVelocity");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new StokesVelocity(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override {
    return new StokesVelocity(*this);
  }

  static constexpr double kG = -9.81;

  template <typename T>
  double CalculateStokesDisplacement(T* cell) {
    auto R = cell->GetDiameter() / 2;
    auto pp = cell->GetDensity();
    auto dt = Simulation::GetActive()->GetParam()->simulation_time_step_;
    auto vel = (4.5) * ((pp - pf_) / u_) * kG * (R * R);
    return vel * dt;
  }

  void Run(Agent* agent) override {
    if (auto* tcell = dynamic_cast<TCell*>(agent)) {
      // Ignore if connected to another cell; it should follow the movements of
      // that cell instead
      if (!tcell->IsConnected()) {
        auto displacement = CalculateStokesDisplacement(tcell);
        tcell->UpdatePosition({0, 0, displacement});
      }
    } else if (auto* monocyte = dynamic_cast<Monocyte*>(agent)) {
      if (monocyte->AtBottom()) {
        return;
      }
      // If a monocyte reaches the bottom of the well, we make it stick to there
      auto min = Simulation::GetActive()->GetParam()->min_bound_;
      if (monocyte->GetPosition()[2] < (min + 1)) {
        monocyte->StickToWellBottom();
      } else {
        auto displacement = CalculateStokesDisplacement(monocyte);
        monocyte->UpdatePosition({0, 0, displacement});
      }
    }
  }

 private:
  double u_ = 1;
  double pf_ = 1;
};

}  // namespace bdm

#endif  // STOKES_VELOCITY_MODULE_H_
