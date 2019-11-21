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
#include "simulation_objects/my_cell.h"

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
    int c = int(r->Uniform(0, 6));
    switch (c) {
      case 0:
        return {1, 0, 0};
      case 1:
        return {-1, 0, 0};
      case 2:
        return {0, 1, 0};
      case 3:
        return {0, -1, 0};
      case 4:
        return {0, 0, 1};
      case 5:
        return {0, 0, -1};
      default:
        return {0, 0, 0};
    }
  }

  void Run(SimObject* so) override {
    if (MyCell* cell = dynamic_cast<MyCell*>(so)) {
      if (!cell->IsConnected()) {
        Double3 direction = GetRandomDirection();
        Double3 vel = direction * velocity_;
        Double3 movement = vel * dt_;
        cell->UpdatePosition(movement);
      }
    }
  }

 private:
  const double dt_ = 1;  // TODO: should be parameterized
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
    int c = int(r->Uniform(0, 4));
    switch (c) {
      case 0:
        return {1, 0, 0};
      case 1:
        return {-1, 0, 0};
      case 2:
        return {0, 1, 0};
      case 3:
        return {0, -1, 0};
      default:
        return {0, 0, 0};
    }
  }

  void Run(SimObject* so) override {
    if (MyCell* cell = dynamic_cast<MyCell*>(so)) {
      if (!cell->IsConnected()) {
        Double3 direction = GetRandomDirection();
        Double3 vel = direction * velocity_;
        Double3 movement = vel * dt_;
        cell->UpdatePosition(movement);
      }
    }
  }

 private:
  const double dt_ = 1;  // TODO: should be parameterized
  double velocity_;
};

}  // namespace bdm

#endif  // RANDOM_WALK_MODULE_H_
