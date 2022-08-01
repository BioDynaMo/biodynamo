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
  Simulation::GetActive()->GetSimulationSpace()->Update();

  container_->rm_ = Simulation::GetActive()->GetResourceManager();
  container_->flat_idx_map_.Update();

  auto* param = Simulation::GetActive()->GetParam();
  unibn::OctreeParams params;
  params.bucketSize = param->unibn_bucketsize;

  impl_->octree_->initialize(*container_, params);
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                                        const Agent& query,
                                        real_t squared_radius) {
  ForEachNeighbor(lambda, query.GetPosition(), squared_radius, &query);
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                                        const Real3& query_position,
                                        real_t squared_radius,
                                        const Agent* query_agent) {
  std::vector<uint32_t> neighbors;
  std::vector<double> distances;

  // Find neighbors
  impl_->octree_->radiusNeighbors<unibn::L2Distance<Real3>>(
      query_position, static_cast<double>(std::sqrt(squared_radius)), neighbors,
      distances);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  int i = 0;
  for (auto& n : neighbors) {
    Agent* nb_so = rm->GetAgent(container_->flat_idx_map_.GetAgentHandle(n));
    if (nb_so != query_agent) {
      lambda(nb_so, static_cast<real_t>(distances[i]));
    }
    i++;
  }
}

void OctreeEnvironment::ForEachNeighbor(Functor<void, Agent*>& lambda,
                                        const Agent& query, void* criteria) {
  Log::Fatal("OctreeEnvironment::ForEachNeighbor",
             "You tried to call a specific ForEachNeighbor in an "
             "environment that does not yet support it.");
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

}  // namespace bdm
