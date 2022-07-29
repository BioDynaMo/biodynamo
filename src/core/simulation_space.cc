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

#include "core/simulation_space.h"
#include "core/util/math.h"
#include "core/functor.h"
#include "core/simulation.h"
#include "core/resource_manager.h"
#include "core/container/shared_data.h"

namespace bdm {
  
// -----------------------------------------------------------------------------
SimulationSpace::SimulationSpace() {
  auto* param = Simulation::GetActive()->GetParam();
  if (param->bound_space) {
    fixed_space_ = true;
    auto min = static_cast<int32_t>(param->min_bound);
    auto max = static_cast<int32_t>(param->max_bound);
    whole_space_ = {min, max, min, max, min, max};
  }
  if (param->interaction_radius != 0) {
    SetInteractionRadius(param->interaction_radius);
  }
  if (fixed_space_ && fixed_interaction_radius_) {
    initialized_ = true;
  }
}

SimulationSpace::SimulationSpace(const SimulationSpace& other)
  : whole_space_(other.whole_space_)
  , fixed_space_(other.fixed_space_)
  , interaction_radius_(other.interaction_radius_)
  , interaction_radius_squared_(other.interaction_radius_squared_)
  , fixed_interaction_radius_(other.fixed_interaction_radius_)
{}

// -----------------------------------------------------------------------------
SimulationSpace::~SimulationSpace() {}

// -----------------------------------------------------------------------------
void SimulationSpace::SetWholeSpace(SimulationSpace::Space space) { 
  whole_space_ = space; 
  fixed_space_ = true;
}

// -----------------------------------------------------------------------------
const SimulationSpace::Space& SimulationSpace::GetWholeSpace() const { return whole_space_; }
// -----------------------------------------------------------------------------
const SimulationSpace::Space& SimulationSpace::GetLocalSpace() const { return whole_space_; }

// -----------------------------------------------------------------------------
void SimulationSpace::SetInteractionRadius(real_t interaction_radius) {
  interaction_radius_ = interaction_radius;
  interaction_radius_squared_ = interaction_radius_ * interaction_radius_;
  fixed_interaction_radius_ = true;
}

// -----------------------------------------------------------------------------
real_t SimulationSpace::GetInteractionRadius() const { return interaction_radius_; } 
// -----------------------------------------------------------------------------
real_t SimulationSpace::GetInteractionRadiusSquared() const { return interaction_radius_squared_; } 

// -----------------------------------------------------------------------------
bool SimulationSpace::operator==(const SimulationSpace& other) {
  return  whole_space_ == other.whole_space_ &&
        fixed_space_ == other.fixed_space_ &&
        interaction_radius_ == other.interaction_radius_ &&
        interaction_radius_squared_ == other.interaction_radius_squared_ &&
        fixed_interaction_radius_ == other.fixed_interaction_radius_;
}

// -----------------------------------------------------------------------------
SimulationSpace& SimulationSpace::operator=(const SimulationSpace& other) {
  if (this != &other) {
    whole_space_ = other.whole_space_; 
    fixed_space_ = other.fixed_space_; 
    interaction_radius_ = other.interaction_radius_; 
    interaction_radius_squared_ = other.interaction_radius_squared_; 
    fixed_interaction_radius_ = other.fixed_interaction_radius_; 
  }
  return *this;
}

namespace {

// -----------------------------------------------------------------------------
struct SimDimensionAndLargestAgentFunctor
    : public Functor<void, Agent*, AgentHandle> {
  using Type = SharedData<real_t>;

  SimDimensionAndLargestAgentFunctor(Type& xmin, Type& xmax, Type& ymin,
                                     Type& ymax, Type& zmin, Type& zmax,
                                     Type& largest, 
                                     bool fixed_space, 
                                     bool fixed_interaction_radius)
      : xmin_(xmin),
        xmax_(xmax),
        ymin_(ymin),
        ymax_(ymax),
        zmin_(zmin),
        zmax_(zmax),
        largest_(largest),
        fixed_space_(fixed_space),
        fixed_interaction_radius_(fixed_interaction_radius) {}

  void operator()(Agent* agent, AgentHandle) override {
    auto tid = omp_get_thread_num();
    if (!fixed_space_) {
      const auto& position = agent->GetPosition();
      // x
      if (position[0] < xmin_[tid]) {
        xmin_[tid] = position[0];
      }
      if (position[0] > xmax_[tid]) {
        xmax_[tid] = position[0];
      }
      // y
      if (position[1] < ymin_[tid]) {
        ymin_[tid] = position[1];
      }
      if (position[1] > ymax_[tid]) {
        ymax_[tid] = position[1];
      }
      // z
      if (position[2] < zmin_[tid]) {
        zmin_[tid] = position[2];
      }
      if (position[2] > zmax_[tid]) {
        zmax_[tid] = position[2];
      }
    }
    
    if (!fixed_interaction_radius_) {
      // largest object
      auto diameter = agent->GetDiameter();
      if (diameter > largest_[tid]) {
        largest_[tid] = diameter;
      }
    }
  }

  Type& xmin_;
  Type& xmax_;
  Type& ymin_;
  Type& ymax_;
  Type& zmin_;
  Type& zmax_;

  Type& largest_;
  bool fixed_space_;
  bool fixed_interaction_radius_;
};

}  // namespace

// -----------------------------------------------------------------------------
/// Calculates what the grid dimensions need to be in order to contain all the
/// agents
void SimulationSpace::DetermineSpaceAndInteractionRadius(
    SimulationSpace::SpaceReal* ret_grid_dimensions) {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  const auto max_threads = omp_get_max_threads();
  // allocate version for each thread - avoid false sharing by using SharedData
  SharedData<real_t> xmin(max_threads, Math::kInfinity);
  SharedData<real_t> xmax(max_threads, -Math::kInfinity);

  SharedData<real_t> ymin(max_threads, Math::kInfinity);
  SharedData<real_t> ymax(max_threads, -Math::kInfinity);

  SharedData<real_t> zmin(max_threads, Math::kInfinity);
  SharedData<real_t> zmax(max_threads, -Math::kInfinity);

  SharedData<real_t> largest(max_threads, 0);

  SimDimensionAndLargestAgentFunctor functor(xmin, xmax, ymin, ymax, zmin,
                                             zmax, largest,
                                             fixed_space_,
                                             fixed_interaction_radius_);
  rm->ForEachAgentParallel(1000, functor);

  // reduce partial results into global one
  real_t& gxmin = (*ret_grid_dimensions)[0];
  real_t& gxmax = (*ret_grid_dimensions)[1];
  real_t& gymin = (*ret_grid_dimensions)[2];
  real_t& gymax = (*ret_grid_dimensions)[3];
  real_t& gzmin = (*ret_grid_dimensions)[4];
  real_t& gzmax = (*ret_grid_dimensions)[5];
  for (int tid = 0; tid < max_threads; tid++) {
    if (!fixed_space_) {
      // x
      if (xmin[tid] < gxmin) {
        gxmin = xmin[tid];
      }
      if (xmax[tid] > gxmax) {
        gxmax = xmax[tid];
      }
      // y
      if (ymin[tid] < gymin) {
        gymin = ymin[tid];
      }
      if (ymax[tid] > gymax) {
        gymax = ymax[tid];
      }
      // z
      if (zmin[tid] < gzmin) {
        gzmin = zmin[tid];
      }
      if (zmax[tid] > gzmax) {
        gzmax = zmax[tid];
      }
    }
    
    if (!fixed_interaction_radius_) {
      // largest object
      if (largest[tid] > interaction_radius_) {
        interaction_radius_ = largest[tid];
      }
    }
  }

  if (!fixed_interaction_radius_) {
    interaction_radius_squared_ = interaction_radius_ * interaction_radius_;
  }
}

  
// -----------------------------------------------------------------------------
void SimulationSpace::RoundOffSpaceDimensions(const SimulationSpace::SpaceReal& real_space) { 
    // Check if conversion can be done without loosing information
    assert(floor(real_space[0]) >= std::numeric_limits<int32_t>::min());
    assert(floor(real_space[2]) >= std::numeric_limits<int32_t>::min());
    assert(floor(real_space[4]) >= std::numeric_limits<int32_t>::min());
    assert(ceil(real_space[1]) <= std::numeric_limits<int32_t>::max());
    assert(ceil(real_space[3]) <= std::numeric_limits<int32_t>::max());
    assert(ceil(real_space[3]) <= std::numeric_limits<int32_t>::max());
    whole_space_[0] = static_cast<int32_t>(floor(real_space[0]));
    whole_space_[2] = static_cast<int32_t>(floor(real_space[2]));
    whole_space_[4] = static_cast<int32_t>(floor(real_space[4]));
    whole_space_[1] = static_cast<int32_t>(ceil(real_space[1]));
    whole_space_[3] = static_cast<int32_t>(ceil(real_space[3]));
    whole_space_[5] = static_cast<int32_t>(ceil(real_space[5]));
}
  
// -----------------------------------------------------------------------------
void SimulationSpace::Update() {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto* param = Simulation::GetActive()->GetParam(); 

  if (fixed_space_ && fixed_interaction_radius_) {
    return;
  }

  if (rm->GetNumAgents() != 0) {
    auto inf = Math::kInfinity;
    SpaceReal tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
    DetermineSpaceAndInteractionRadius(&tmp_dim);
    if (!fixed_space_) {
      RoundOffSpaceDimensions(tmp_dim);
    }
    initialized_ = true;
  } else if (!initialized_) {
      Log::Fatal(
          "SimulationSpace",
          "We cannot determine the simulation space and maximum interaction radius automatically, because the simulation never contained any agents.\n"
          "Please add agents to the simulation or set these values e.g. using SimulationSpace::SetWholeSpace and SimulationSpace::SetInteractionRadius.\n"
          "Alternatively, you can set the parameters Param::bound_space > 1, Param::min_bound, Param::max_bound, and Param::interaction_radius");
  }
  // if there are no agents in the simulation and the simulation space is 
  // initialized, keep the values from before
}

}  // namespace bdm
