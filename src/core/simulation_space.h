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
#include "core/param/param.h"
#include "core/real_t.h"
#include "core/simulation.h"

namespace bdm {

/// This class contains the logic to determine the simulation space and the
/// interaction radius of agents.
/// These values can be defined in three ways:\n
/// First, by setting `Param::bound_space`,
/// `Param::min_bound`, `Param::max_bound` and `Param::interaction_radius`.
/// Second, by using the member functions `SetWholeSpace` and
/// `SetInteractionRadius`.
/// Third, by determining the simulation space and interaction radius
/// automatically, if `Param::bound_space == kOpen` and the first two options
/// were not used.\n
/// Automatic space: the mechanism iterates over all agents and determines min
/// and max values for the x, y, and z dimension. Afterwards it subtracts the
/// interaction radius from min and adds it to max.\n Automatic interaction
/// radius: the mechanism iterates over all agents to find the largest one. The
/// interaction radius is set to `2 * radius` of the largest agent.
class SimulationSpace {
 public:
  using Space = MathArray<int32_t, 6>;
  using SpaceReal = MathArray<real_t, 6>;

  SimulationSpace();

  SimulationSpace(const SimulationSpace& other);

  virtual ~SimulationSpace();

  // Setting the whole simulation space will disable the automatic determination
  void SetWholeSpace(const Space& space);

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
class DistributedSimSpace : public SimulationSpace {
 public:
  virtual ~DistributedSimSpace() {}
  virtual const Space& GetLocalSpace() const;
  void SetLocalSpace(const SimulationSpace::Space& space);

 private:
  SimulationSpace::Space local_space_;

  virtual void Update();
};

}  // namespace bdm

#endif  // CORE_SIMULATION_SPACE_H_
