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

#include <algorithm>

#include "core/environment/kd_tree_environment.h"

#include <nanoflann.hpp>

namespace bdm {

using nanoflann::KDTreeSingleIndexAdaptor;
using nanoflann::KDTreeSingleIndexAdaptorParams;
using nanoflann::L2_Simple_Adaptor;

typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<double, NanoFlannAdapter>,
                                 NanoFlannAdapter, 3, uint64_t>
    bdm_kd_tree_t;

struct KDTreeEnvironment::NanoflannImpl {
  bdm_kd_tree_t* index_ = nullptr;
};

KDTreeEnvironment::KDTreeEnvironment() {
  auto* param = Simulation::GetActive()->GetParam();
  nf_adapter_ = new NanoFlannAdapter();
  impl_ = std::unique_ptr<KDTreeEnvironment::NanoflannImpl>(
      new KDTreeEnvironment::NanoflannImpl());
  impl_->index_ = new bdm_kd_tree_t(
      3, *nf_adapter_, KDTreeSingleIndexAdaptorParams(param->nanoflann_depth));
}

KDTreeEnvironment::~KDTreeEnvironment() {
  delete impl_->index_;
  delete nf_adapter_;
}

void KDTreeEnvironment::Update() {
  nf_adapter_->rm_ = Simulation::GetActive()->GetResourceManager();

  // Update the flattened indices map
  nf_adapter_->flat_idx_map_.Update();
  if (nf_adapter_->rm_->GetNumAgents() != 0) {
    Clear();
    auto inf = Math::kInfinity;
    std::array<double, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
    CalcSimDimensionsAndLargestAgent(&tmp_dim);
    RoundOffGridDimensions(tmp_dim);
    CheckGridGrowth();
    impl_->index_->buildIndex();
  } else {
    // There are no sim objects in this simulation
    auto* param = Simulation::GetActive()->GetParam();

    bool uninitialized = impl_->index_->m_size == 0;
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
          "KDtreeEnvironment",
          "You tried to initialize an empty simulation without bound space. "
          "Therefore we cannot determine the size of the simulation space. "
          "Please add simulation objects, or set Param::bound_space_, "
          "Param::min_bound_, and Param::max_bound_.");
    }
  }
}

void KDTreeEnvironment::ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                                        const Agent& query, void* criteria) {
  double squared_radius = *static_cast<double*>(criteria);
  std::vector<std::pair<uint64_t, double>> neighbors;

  nanoflann::SearchParams params;
  params.sorted = false;

  const auto& position = query.GetPosition();

  // calculate neighbors
  impl_->index_->radiusSearch(&position[0], squared_radius, neighbors, params);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (auto& n : neighbors) {
    Agent* nb_so =
        rm->GetAgent(nf_adapter_->flat_idx_map_.GetAgentHandle(n.first));
    if (nb_so != &query) {
      lambda(nb_so, n.second);
    }
  }
}

std::array<int32_t, 6> KDTreeEnvironment::GetDimensions() const {
  return grid_dimensions_;
}

std::array<int32_t, 2> KDTreeEnvironment::GetDimensionThresholds() const {
  return threshold_dimensions_;
}

LoadBalanceInfo* KDTreeEnvironment::GetLoadBalanceInfo() {
  Log::Fatal("KDTreeEnvironment::GetLoadBalanceInfo",
             "You tried to call GetLoadBalanceInfo in an environment that does "
             "not support it.");
  return nullptr;
}

Environment::NeighborMutexBuilder*
KDTreeEnvironment::GetNeighborMutexBuilder() {
  return nullptr;
};

void KDTreeEnvironment::Clear() {
  int32_t inf = std::numeric_limits<int32_t>::max();
  grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
  threshold_dimensions_ = {inf, -inf};
}

void KDTreeEnvironment::RoundOffGridDimensions(
    const std::array<double, 6>& grid_dimensions) {
  grid_dimensions_[0] = floor(grid_dimensions[0]);
  grid_dimensions_[2] = floor(grid_dimensions[2]);
  grid_dimensions_[4] = floor(grid_dimensions[4]);
  grid_dimensions_[1] = ceil(grid_dimensions[1]);
  grid_dimensions_[3] = ceil(grid_dimensions[3]);
  grid_dimensions_[5] = ceil(grid_dimensions[5]);
}

void KDTreeEnvironment::CheckGridGrowth() {
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
