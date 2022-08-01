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

#ifndef CORE_ENVIRONMENT_OCTREE_ENVIRONMENT_
#define CORE_ENVIRONMENT_OCTREE_ENVIRONMENT_

#include "core/agent/agent_uid.h"
#include "core/container/agent_flat_idx_map.h"
#include "core/container/math_array.h"
#include "core/environment/environment.h"
#include "core/simulation.h"

namespace bdm {

/// This class acts as a contiguous container of simulation object positions for
/// the Unibn octree API
class AgentContainer {
 public:
  AgentContainer() { rm_ = Simulation::GetActive()->GetResourceManager(); }

  size_t size() const { return rm_->GetNumAgents(); }

  const Real3& operator[](size_t idx) const {
    AgentHandle ah = flat_idx_map_.GetAgentHandle(idx);
    return rm_->GetAgent(ah)->GetPosition();
  }

  AgentFlatIdxMap flat_idx_map_;
  ResourceManager* rm_ = nullptr;
};

class OctreeEnvironment : public Environment {
 public:
  struct UnibnImpl;

  OctreeEnvironment();

  ~OctreeEnvironment();

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  NeighborMutexBuilder* GetNeighborMutexBuilder() override;

  void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                       const Agent& query, real_t squared_radius) override;

  void ForEachNeighbor(Functor<void, Agent*>& lambda, const Agent& query,
                       void* criteria) override;

  void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                       const Real3& query_position, real_t squared_radius,
                       const Agent* query_agent = nullptr) override;

 protected:
  void UpdateImplementation() override;

 private:
  // Hide unibn-specific types from header (pimpl idiom)
  std::unique_ptr<UnibnImpl> impl_;
  AgentContainer* container_ = nullptr;
  /// Cuboid which contains all agents
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  MathArray<int32_t, 6> grid_dimensions_;
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_OCTREE_ENVIRONMENT_
