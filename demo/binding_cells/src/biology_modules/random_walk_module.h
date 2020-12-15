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

#include "core/behavior/behavior.h"

namespace bdm {

/// Make a simulation object move at a constant velocity towards the direction
struct RandomWalk : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomWalk, Behavior, 1);

 public:
  RandomWalk(double v = 1) : velocity_(v) { AlwaysCopyToNew(); }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = event.existing_behavior;
    if (RandomWalk* gdbm = dynamic_cast<RandomWalk*>(other)) {
      velocity_ = gdbm->velocity_;
    } else {
      Log::Fatal("RandomWalk::EventConstructor",
                 "other was not of type RandomWalk");
    }
  }

  Double3 GetRandomDirection() {
    auto* r = Simulation::GetActive()->GetRandom();
    Double3 random_vector = r->UniformArray<3>(-1, 1);
    return random_vector.Normalize();
  }

  void Run(Agent* agent) override {
    if (auto* monocyte = dynamic_cast<Monocyte*>(agent)) {
      if (monocyte->AtBottom()) {
        return;
      }
    }
    if (auto* cell = dynamic_cast<Cell*>(agent)) {
      Double3 direction = GetRandomDirection();
      Double3 vel = direction * velocity_;
      auto dt = Simulation::GetActive()->GetParam()->simulation_time_step;
      cell->UpdatePosition(vel * dt);
    }
  }

 private:
  double velocity_;
};

/// Make a simulation object move at a constant velocity towards the direction
struct RandomWalkXY : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomWalkXY, Behavior, 1);

 public:
  RandomWalkXY(double v = 1) : velocity_(v) { AlwaysCopyToNew(); }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = event.existing_behavior;
    if (RandomWalkXY* gdbm = dynamic_cast<RandomWalkXY*>(other)) {
      velocity_ = gdbm->velocity_;
    } else {
      Log::Fatal("RandomWalkXY::EventConstructor",
                 "other was not of type RandomWalkXY");
    }
  }

  Double3 GetRandomDirection() {
    auto* r = Simulation::GetActive()->GetRandom();
    Double3 random_vector;
    random_vector[0] = r->Uniform(-1, 1);
    random_vector[1] = r->Uniform(-1, 1);
    random_vector[2] = 0;
    return random_vector.Normalize();
  }

  void Run(Agent* agent) override {
    if (auto* cell = dynamic_cast<Cell*>(agent)) {
      Double3 direction = GetRandomDirection();
      Double3 vel = direction * velocity_;
      auto dt = Simulation::GetActive()->GetParam()->simulation_time_step;
      cell->UpdatePosition(vel * dt);
    }
  }

 private:
  double velocity_;
};

}  // namespace bdm

#endif  // RANDOM_WALK_MODULE_H_
