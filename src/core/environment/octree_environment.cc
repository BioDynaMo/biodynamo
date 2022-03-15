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

#include <algorithm>

#include "core/environment/octree_environment.h"

#include "unibn_octree.h"

namespace bdm {

struct OctreeEnvironment::UnibnImpl {
  unibn::Octree<Real3, AgentContainer>* octree_ = nullptr;
};

OctreeEnvironment::OctreeEnvironment() {
  impl_ = std::unique_ptr<OctreeEnvironment::UnibnImpl>(
      new OctreeEnvironment::UnibnImpl());
  impl_->octree_ = new unibn::Octree<Real3, AgentContainer>();
  container_ = new AgentContainer();
}

OctreeEnvironment::~OctreeEnvironment() {
  delete impl_->octree_;
  delete container_;
}

void OctreeEnvironment::UpdateImplementation() {
  container_->rm_ = Simulation::GetActive()->GetResourceManager();
  auto* param = Simulation::GetActive()->GetParam();

  // Update the flattened indices map
  
  if (container_->rm_->GetNumAgents() != 0) {
    Clear();
    auto inf = Math::kInfinity;
    std::array<real, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
    CalcSimDimensionsAndLargestAgent(&tmp_dim);
    RoundOffGridDimensions(tmp_dim);
    CheckGridGrowth();

    unibn::OctreeParams params;
    params.bucketSize = param->unibn_bucketsize;

    impl_->octree_->initialize(*container_, params);
  } else {
    // There are no sim objects in this simulation
    auto* param = Simulation::GetActive()->GetParam();

    bool uninitialized = impl_->octree_ == nullptr;
    if (uninitialized && param->bound_space) {
      // Simulation has never had any simulation objects
      // Initialize grid dimensions with `Param::min_bound_` and
      // `Param::max_bound_`
      // This is required for the DiffusionGrid
      int min = param->min_bound;
      int max = param->max_bound;
      grid_dimensions_ = {min, max, min, max, min, max};
      threshold_dimensions_ = {min, max};
      has_grown_ = true;
    } else if (!uninitialized) {
      // all simulation objects have been removed in the last iteration
      // grid state remains the same, but we have to set has_grown_ to false
      // otherwise the DiffusionGrid will attempt to resize
      has_grown_ = false;
    } else {
      Log::Fatal(
          "OctreeEnvironment",
          "You tried to initialize an empty simulation without bound space. "
          "Therefore we cannot determine the size of the simulation space. "
          "Please add simulation objects, or set Param::bound_space_, "
          "Param::min_bound_, and Param::max_bound_.");
    }
  }
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real>& lambda,
                                        const Agent& query,
                                        real squared_radius) {
  ForEachNeighbor(lambda, query.GetPosition(), squared_radius, &query);
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real>& lambda,
                                        const Real3& query_position,
                                        real squared_radius,
                                        const Agent* query_agent) {
  // TODO
  // std::vector<uint32_t> neighbors;
  // std::vector<real> distances;
  // 
  // // Find neighbors
  // impl_->octree_->radiusNeighbors<unibn::L2Distance<Real3>>(
  //     query_position, static_cast<double>(std::sqrt(squared_radius)), neighbors, distances);
  // 
  // auto* rm = Simulation::GetActive()->GetResourceManager();
  // int i = 0;
  // for (auto& n : neighbors) {
  //   Agent* nb_so = rm->GetAgent(container_->flat_idx_map_.GetAgentHandle(n));
  //   if (nb_so != query_agent) {
  //     lambda(nb_so, distances[i]);
  //   }
  //   i++;
  // }
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*>& lambda,
                                        const Agent& query, void* criteria) {
  Log::Fatal("OctreeEnvironment::ForEachNeighbor",
             "You tried to call a specific ForEachNeighbor in an "
             "environment that does not yet support it.");
}

std::array<int32_t, 6> OctreeEnvironment::GetDimensions() const {
  return grid_dimensions_;
}

std::array<int32_t, 2> OctreeEnvironment::GetDimensionThresholds() const {
  return threshold_dimensions_;
}

LoadBalanceInfo* OctreeEnvironment::GetLoadBalanceInfo() {
  Log::Fatal("OctreeEnvironment::GetLoadBalanceInfo",
             "You tried to call GetLoadBalanceInfo in an environment that does "
             "not support it.");
  return nullptr;
}

Environment::NeighborMutexBuilder*
OctreeEnvironment::GetNeighborMutexBuilder() {
  return nullptr;
};

void OctreeEnvironment::Clear() {
  int32_t inf = std::numeric_limits<int32_t>::max();
  grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
  threshold_dimensions_ = {inf, -inf};
}

void OctreeEnvironment::RoundOffGridDimensions(
    const std::array<real, 6>& grid_dimensions) {
  grid_dimensions_[0] = floor(grid_dimensions[0]);
  grid_dimensions_[2] = floor(grid_dimensions[2]);
  grid_dimensions_[4] = floor(grid_dimensions[4]);
  grid_dimensions_[1] = ceil(grid_dimensions[1]);
  grid_dimensions_[3] = ceil(grid_dimensions[3]);
  grid_dimensions_[5] = ceil(grid_dimensions[5]);
}

void OctreeEnvironment::CheckGridGrowth() {
  // Determine if the grid dimensions have changed (changed in the sense that
  // the grid has grown outwards)
  auto min_gd =
      *std::min_element(grid_dimensions_.begin(), grid_dimensions_.end());
  auto max_gd =
      *std::max_element(grid_dimensions_.begin(), grid_dimensions_.end());
  if (min_gd < threshold_dimensions_[0]) {
    threshold_dimensions_[0] = min_gd;
    has_grown_ = true;
  }
  if (max_gd > threshold_dimensions_[1]) {
    threshold_dimensions_[1] = max_gd;
    has_grown_ = true;
  }
}

}  // namespace bdm
