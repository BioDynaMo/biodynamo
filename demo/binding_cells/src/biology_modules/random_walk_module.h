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
#ifndef RANDOM_WALK_MODULE_H_
#define RANDOM_WALK_MODULE_H_

#include "biodynamo.h"

namespace bdm {

/// Make a simulation object move at a constant velocity towards the direction
struct RandomWalk : public BaseBiologyModule {
 public:
  RandomWalk(double v = 1) : BaseBiologyModule(gAllEventIds), velocity_(v) {}

  RandomWalk(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (RandomWalk* gdbm = dynamic_cast<RandomWalk*>(other)) {
      velocity_ = gdbm->velocity_;
    } else {
      Log::Fatal("RandomWalk::EventConstructor",
                 "other was not of type RandomWalk");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new RandomWalk(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override { return new RandomWalk(*this); }

  Double3 GetRandomDirection() {
    auto* r = Simulation::GetActive()->GetRandom();
    Double3 random_vector = r->UniformArray<3>(-1, 1);
    return random_vector.Normalize();
  }

  void Run(SimObject* so) override {
    if (auto* monocyte = dynamic_cast<Monocyte*>(so)) {
      if (monocyte->AtBottom()) {
        return;
      }
    }
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      Double3 direction = GetRandomDirection();
      Double3 vel = direction * velocity_;
      auto dt = Simulation::GetActive()->GetParam()->simulation_time_step_;
      cell->UpdatePosition(vel * dt);
    }
  }

 private:
  double velocity_;
};

/// Make a simulation object move at a constant velocity towards the direction
struct RandomWalkXY : public BaseBiologyModule {
 public:
  RandomWalkXY(double v = 1) : BaseBiologyModule(gAllEventIds), velocity_(v) {}

  RandomWalkXY(const Event& event, BaseBiologyModule* other,
               uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (RandomWalkXY* gdbm = dynamic_cast<RandomWalkXY*>(other)) {
      velocity_ = gdbm->velocity_;
    } else {
      Log::Fatal("RandomWalkXY::EventConstructor",
                 "other was not of type RandomWalkXY");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new RandomWalkXY(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override {
    return new RandomWalkXY(*this);
  }

  Double3 GetRandomDirection() {
    auto* r = Simulation::GetActive()->GetRandom();
    Double3 random_vector;
    random_vector[0] = r->Uniform(-1, 1);
    random_vector[1] = r->Uniform(-1, 1);
    random_vector[2] = 0;
    return random_vector.Normalize();
  }

  void Run(SimObject* so) override {
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      Double3 direction = GetRandomDirection();
      Double3 vel = direction * velocity_;
      auto dt = Simulation::GetActive()->GetParam()->simulation_time_step_;
      cell->UpdatePosition(vel * dt);
    }
  }

 private:
  double velocity_;
};

}  // namespace bdm

#endif  // RANDOM_WALK_MODULE_H_
