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

  const Double3& operator[](size_t idx) const {
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

  std::array<int32_t, 6> GetDimensions() const override;

  std::array<int32_t, 2> GetDimensionThresholds() const override;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  NeighborMutexBuilder* GetNeighborMutexBuilder() override;

  void Clear() override;

  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  void ForEachNeighbor(Functor<void, Agent*>& lambda, const Agent& query,
                       void* criteria) override;

  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Double3& query_position, double squared_radius,
                       const Agent* query_agent = nullptr) override;

 protected:
  void UpdateImplementation() override;

 private:
  // Hide unibn-specific types from header (pimpl idiom)
  std::unique_ptr<UnibnImpl> impl_;
  AgentContainer* container_ = nullptr;
  /// Cube which contains all simulation objects
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  /// Stores the min / max dimension value that need to be surpassed in order
  /// to trigger a diffusion grid change
  std::array<int32_t, 2> threshold_dimensions_;

  void RoundOffGridDimensions(const std::array<double, 6>& grid_dimensions);

  void CheckGridGrowth();
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_OCTREE_ENVIRONMENT_
