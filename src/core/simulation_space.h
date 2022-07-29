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
#include "core/simulation.h"
#include "core/param/param.h"

namespace bdm {

/// This class contains the logic to define the simulation space and the 
/// interaction radius of agents. 
// TODO(lukas) documentation
class SimulationSpace {
 public:
  using Space = MathArray<int32_t, 6>;
  using SpaceReal = MathArray<real_t, 6>;

  SimulationSpace();

  SimulationSpace(const SimulationSpace& other);

  virtual ~SimulationSpace();

  // Setting the whole simulation space will disable the automatic determination
  void SetWholeSpace(Space space);

  const Space& GetWholeSpace() const;

  virtual const Space& GetLocalSpace() const;

  void SetInteractionRadius(real_t interaction_radius);

  real_t GetInteractionRadius() const;
  real_t GetInteractionRadiusSquared() const;

  virtual bool operator==(const SimulationSpace& other);

  virtual SimulationSpace& operator=(const SimulationSpace& other);
  
  /// Updates the simulation space and interaction radius if these variables
  /// are determined automatically.
  virtual void Update();

 protected:
  bool initialized_ = false;
  Space whole_space_;
  bool fixed_space_ = false;
  real_t interaction_radius_ = 0.0;
  real_t interaction_radius_squared_ = 0.0;
  bool fixed_interaction_radius_ = false;

 private:
  /// Determines what the simulaiton space need to be in order to contain 
  /// all the agents if `Param::bound_space > 0 && !fixed_space_`.\n
  /// Determines the interaction radius based on the larget agent in the
  /// simulation 
  void DetermineSpaceAndInteractionRadius(SpaceReal* space);

  // Convert SpaceReal to Space and set whole_space_
  void RoundOffSpaceDimensions(const SpaceReal& real_space);
};

// FIXME move to separate file
struct DistributedSimSpace : public SimulationSpace {
  virtual ~DistributedSimSpace() {}
  MathArray<double, 6> local_space;
};

}  // namespace bdm

#endif  // CORE_SIMULATION_SPACE_H_
