// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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
#ifndef CORE_SIMULATION_SPACE_H_
#define CORE_SIMULATION_SPACE_H_

#include "core/container/math_array.h"
#include "core/real_t.h"

namespace bdm {

/// This class contains the logic to define the simulation space and the 
/// interaction radius of agents. 
// TODO(lukas) documentation
// TODO(lukas) move definitions to cc file
class SimulationSpace {
 public:
  using Space = MathArray<real_t, 6>;

  SimulationSpace() {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space) {
      fixed_space_ = true;
      auto min = param->min_bound;
      auto max = param->max_bound;
      whole_space_ = {min, max, min, max, min, max};
    }
  }

  SimulationSpace(const SimulationSpace& other)
    : whole_space_(other.whole_space_)
    , fixed_space_(other.fixed_space_)
    , interaction_radius_(other.interaction_radius_)
    , interaction_radius_squared_(other.interaction_radius_squared_)
    , fixed_interaction_radius_(other.fixed_interaction_radius_)
  {}

  virtual ~SimulationSpace() {}

  // Setting the whole simulation space will disable the automatic determination
  void SetWholeSpace(Space space) { 
    whole_space_ = space; 
    fixed_space_ = true;
  }

  const Space& GetWholeSpace() const { return whole_space_; }
  virtual const Space& GetLocalSpace() const { return whole_space_; }

  // Setting the interaction space will disable the automatic determination.
  void SetInteractionRadius(real_t interaction_radius) {
    interaction_radius_ = interaction_radius;
    interaction_radius_squared_ = interaction_radius_ * interaction_radius_;
    fixed_interaction_radius_ = true;
  }

  real_t GetInteractionRadius() const { return interaction_radius_; } 
  real_t GetInteractionRadiusSquared() const { return interaction_radius_squared_; } 

  virtual bool operator==(const SimulationSpace& other) {
    return  whole_space_ == other.whole_space_ &&
          fixed_space_ == other.fixed_space_ &&
          interaction_radius_ == other.interaction_radius_ &&
          interaction_radius_squared_ == other.interaction_radius_squared_ &&
          fixed_interaction_radius_ == other.fixed_interaction_radius_;
  }

  virtual SimulationSpace& operator=(const SimulationSpace& other) {
    if (this != &other) {
      whole_space_ = other.whole_space_; 
      fixed_space_ = other.fixed_space_; 
      interaction_radius_ = other.interaction_radius_; 
      interaction_radius_squared_ = other.interaction_radius_squared_; 
      fixed_interaction_radius_ = other.fixed_interaction_radius_; 
    }
    return *this;
  }

 protected:
  Space whole_space_;
  bool fixed_space_ = false;
  real_t interaction_radius_ = 1.0;
  real_t interaction_radius_squared_ = 1.0;
  bool fixed_interaction_radius_ = false;

  void Update() {
    UpdateWholeSimulationSpace(); 
    UpdateInteractionRadius();
  }

  virtual void UpdateWholeSimulationSpace() {
  }
  
  virtual void UpdateInteractionRadius() {
  }

};

// FIXME move to separate file
struct DistributedSimSpace : public SimulationSpace {
  virtual ~DistributedSimSpace() {}
  MathArray<double, 6> local_space;
};

}  // namespace bdm

#endif  // CORE_SIMULATION_SPACE_H_
