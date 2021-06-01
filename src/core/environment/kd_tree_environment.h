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

#ifndef CORE_ENVIRONMENT_KD_TREE_ENVIRONMENT_
#define CORE_ENVIRONMENT_KD_TREE_ENVIRONMENT_

#include "core/container/agent_flat_idx_map.h"
#include "core/container/math_array.h"
#include "core/environment/environment.h"
#include "core/simulation.h"

namespace bdm {

struct NanoFlannAdapter {
  using coord_t = Double3;
  using idx_t = uint64_t;

  NanoFlannAdapter() { rm_ = Simulation::GetActive()->GetResourceManager(); }

  /// Must return the number of data points
  inline size_t kdtree_get_point_count() const {
    return Simulation::GetActive()->GetResourceManager()->GetNumAgents();
  }

  /// Returns the distance between the vector "p1[0:size-1]" and the data point
  /// with index "idx_p2" stored in the class:
  inline double kdtree_distance(const coord_t& p1, const idx_t idx_p2,
                                size_t /*size*/) const {
    AgentHandle ah = flat_idx_map_.GetAgentHandle(idx_p2);
    auto& p2 = rm_->GetAgent(ah)->GetPosition();
    return p1 * p2;
  }

  /// Returns the dim'th component of the idx'th point in the class:
  /// Since this is inlined and the "dim" argument is typically an immediate
  /// value, the "if/else's" are actually solved at compile time.
  inline double kdtree_get_pt(const idx_t idx, int dim) const {
    AgentHandle ah = flat_idx_map_.GetAgentHandle(idx);
    auto& pos = rm_->GetAgent(ah)->GetPosition();
    return pos[dim];
  }

  /// Optional bounding-box computation: return false to default to a standard
  /// bbox computation loop.
  ///   Return true if the BBOX was already computed by the class and returned
  ///   in
  ///   "bb" so it can be avoided to redo it again.
  ///   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3
  ///   for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox(BBOX&) const {
    return false;
  }

  AgentFlatIdxMap flat_idx_map_;
  ResourceManager* rm_ = nullptr;
};

class KDTreeEnvironment : public Environment {
 public:
  struct NanoflannImpl;

  KDTreeEnvironment();

  ~KDTreeEnvironment();

  void Update() override;

  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  std::array<int32_t, 6> GetDimensions() const override;

  std::array<int32_t, 2> GetDimensionThresholds() const override;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  NeighborMutexBuilder* GetNeighborMutexBuilder() override;

  void Clear() override;

 private:
  // Hide nanoflann-specific types from header (pimpl idiom)
  std::unique_ptr<NanoflannImpl> impl_;
  /// Cube which contains all simulation objects
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  /// Stores the min / max dimension value that need to be surpassed in order
  /// to trigger a diffusion grid change
  std::array<int32_t, 2> threshold_dimensions_;
  NanoFlannAdapter* nf_adapter_ = nullptr;

  void RoundOffGridDimensions(const std::array<double, 6>& grid_dimensions);

  void CheckGridGrowth();
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_KD_TREE_ENVIRONMENT_
