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

#include "core/environment/kd_tree_environment.h"
#include "core/simulation_space.h"

#include "nanoflann/nanoflann.hpp"

namespace bdm {

using nanoflann::KDTreeSingleIndexAdaptor;
using nanoflann::KDTreeSingleIndexAdaptorParams;
using nanoflann::L2_Simple_Adaptor;

typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<real_t, NanoFlannAdapter>,
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

void KDTreeEnvironment::UpdateImplementation() {
  Simulation::GetActive()->GetSimulationSpace()->Update();

  nf_adapter_->rm_ = Simulation::GetActive()->GetResourceManager();

  // Update the flattened indices map
  nf_adapter_->flat_idx_map_.Update();
  
  impl_->index_->buildIndex();
}

void KDTreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                                        const Agent& query,
                                        real_t squared_radius) {
  ForEachNeighbor(lambda, query.GetPosition(), squared_radius, &query);
}

void KDTreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                                        const Real3& query_position,
                                        real_t squared_radius,
                                        const Agent* query_agent) {
  std::vector<std::pair<uint64_t, real_t>> neighbors;

  nanoflann::SearchParams params;
  params.sorted = false;

  // calculate neighbors
  impl_->index_->radiusSearch(&query_position[0], squared_radius, neighbors,
                              params);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (auto& n : neighbors) {
    Agent* nb_so =
        rm->GetAgent(nf_adapter_->flat_idx_map_.GetAgentHandle(n.first));
    if (nb_so != query_agent) {
      lambda(nb_so, n.second);
    }
  }
}

void KDTreeEnvironment::ForEachNeighbor(Functor<void, Agent*>& lambda,
                                        const Agent& query, void* criteria) {
  Log::Fatal("KDTreeEnvironment::ForEachNeighbor",
             "You tried to call a specific ForEachNeighbor in an "
             "environment that does not yet support it.");
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

}  // namespace bdm
