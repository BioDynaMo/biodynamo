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
#ifndef CONSTANT_DISPLACEMENT_MODULE_H_
#define CONSTANT_DISPLACEMENT_MODULE_H_

#include "agents/monocyte.h"
#include "core/behavior/behavior.h"

namespace bdm {

/// @brief      Calculates the squared euclidian distance between two points
///             in 3D
///
/// @param[in]  pos1  Position of the first point
/// @param[in]  pos2  Position of the second point
///
/// @return     The distance between the two points
///
inline double SquaredEuclideanDistance(const Double3& pos1,
                                       const Double3& pos2) {
  const double dx = pos2[0] - pos1[0];
  const double dy = pos2[1] - pos1[1];
  const double dz = pos2[2] - pos1[2];
  return (dx * dx + dy * dy + dz * dz);
}

/// Make a simulation object move at a constant velocity towards the direction
struct ConstantDisplace : public Behavior {
  BDM_BEHAVIOR_HEADER(ConstantDisplace, Behavior, 1);

 public:
  ConstantDisplace(double v = 1, Double3 goal_position = {0, 0, 0})
      : velocity_(v), goal_position_(goal_position) {
    AlwaysCopyToNew();
  }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = event.existing_behavior;
    if (ConstantDisplace* gdbm = dynamic_cast<ConstantDisplace*>(other)) {
      velocity_ = gdbm->velocity_;
    } else {
      Log::Fatal("ConstantDisplace::EventConstructor",
                 "other was not of type ConstantDisplace");
    }
  }

  void SetGoalPosition(Double3 new_goal) {
    goal_position_ = new_goal;
    reached_goal_ = false;
  }

  void Run(Agent* agent) override {
    if (!reached_goal_) {
      if (auto* cell = dynamic_cast<Monocyte*>(agent)) {
        auto sq_distance =
            SquaredEuclideanDistance(agent->GetPosition(), goal_position_);
        if (sq_distance > eps_) {
          Double3 direction = goal_position_ - cell->GetPosition();
          direction.Normalize();
          Double3 vel = direction * velocity_;
          Double3 movement = vel * dt_;
          cell->UpdatePosition(movement);
        } else {  // move the last remaining bit
          cell->SetPosition(goal_position_);
          reached_goal_ = true;
        }
      }
    }
  }

 private:
  const double dt_ = 1;  // TODO: should be parameterized
  const double eps_ = 5;
  bool reached_goal_ = false;
  double velocity_;
  Double3 goal_position_;
};

}  // namespace bdm

#endif  // CONSTANT_DISPLACEMENT_MODULE_H_
