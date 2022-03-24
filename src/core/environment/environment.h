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

#ifndef CORE_ENVIRONMENT_ENVIRONMENT_H_
#define CORE_ENVIRONMENT_ENVIRONMENT_H_

#ifdef BDM_USE_OMP
#include <omp.h>
#endif  // BDM_USE_OMP
#include <cassert>
#include <mutex>
#include <vector>
#include "core/agent/agent.h"
#include "core/container/math_array.h"
#include "core/functor.h"
#include "core/load_balance_info.h"
#include "core/resource_manager.h"
#include "core/util/thread_info.h"

namespace bdm {

class Environment {
 private:
  // Flag that indicates if the environment is up to date with the simulation.
  // E.g. load balancing can result in an environment that does no longer
  // describe the actual state of the simulation.
  bool out_of_sync_ = true;

 public:
  virtual ~Environment() = default;

  /// This function informs the environment that it is no longer up to date and
  /// that the state of the simulation might not be reflected correctly in the
  /// current environment. For instance, the load balancing operation causes
  /// such a synchronization issue and therefore calls this member function.
  void MarkAsOutOfSync() { out_of_sync_ = true; }

  /// Updates the environment if it is marked as out_of_sync_. This function
  /// should not be called in parallel regions for performance reasons.
  void Update() {
#ifdef BDM_USE_OMP
    assert(!omp_in_parallel() && "Update called in parallel region.");
#endif  // BDM_USE_OMP
    if (out_of_sync_) {
      UpdateImplementation();
      out_of_sync_ = false;
    }
  }

  /// Updates the environment. Prefer Update() for implementations.
  void ForcedUpdate() {
    MarkAsOutOfSync();
    Update();
  }

  /// Iterates over all neighbors of a query agent that appear in a distance of
  /// less than sqrt(squared_radius). Typically, the distance is computed as the
  /// Euclidean distance in 3D environments.
  virtual void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                               const Agent& query, double squared_radius) = 0;

  /// Iterates over all neighbors in an environment that suffices the given
  /// `criteria`. The `criteria` is type-erased to facilitate for different
  /// criteria for different environments. Check the documentation of an
  /// environment to know the criteria data type
  virtual void ForEachNeighbor(Functor<void, Agent*>& lambda,
                               const Agent& query, void* criteria) = 0;

  /// Iterates over all neighbors of a query position that appear in a
  /// distance of less than sqrt(squared_radius). Typically, the distance is
  /// computed as the Euclidean distance in 3D environments. Typically iterating
  /// over the neighbors of a certain agent or a certain position are fairly
  /// similar tasks. If you implement an environment, the optional argument
  /// agent_query can be used to to treat both cases in the same function, keep
  /// the implementation lean, and avoid redundant code.
  virtual void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                               const Double3& query_position,
                               double squared_radius,
                               const Agent* query_agent = nullptr) = 0;

  virtual void Clear() = 0;

  virtual std::array<int32_t, 6> GetDimensions() const = 0;

  virtual std::array<int32_t, 2> GetDimensionThresholds() const = 0;

  /// Return the size of the largest agent
  double GetLargestAgentSize() const { return largest_object_size_; };
  double GetLargestAgentSizeSquared() const {
    return largest_object_size_squared_;
  };

  virtual LoadBalanceInfo* GetLoadBalanceInfo() = 0;

  /// This class ensures thread-safety for the case
  /// that an agent modifies its neighbors.
  class NeighborMutexBuilder {
   public:
    /// The NeighborMutex class is a synchronization primitive that can be
    /// used to protect agents data from being simultaneously accessed by
    /// multiple threads.
    class NeighborMutex {
     public:
      virtual ~NeighborMutex() = default;
      virtual void lock(){};    // NOLINT
      virtual void unlock(){};  // NOLINT
    };

    virtual ~NeighborMutexBuilder() = default;
    virtual NeighborMutex* GetMutex(uint64_t box_idx) = 0;
  };

  /// Returns the `NeighborMutexBuilder`. The client uses it to create a
  /// `NeighborMutex`.
  virtual NeighborMutexBuilder* GetNeighborMutexBuilder() = 0;

  bool HasGrown() const { return has_grown_; }

 protected:
  bool has_grown_ = false;
  /// The size of the largest object in the simulation
  double largest_object_size_ = 0.0;
  double largest_object_size_squared_ = 0.0;

  /// Member function that is called by Update() and ForcedUpdate(). Pure
  /// virtual.
  virtual void UpdateImplementation() = 0;

  struct SimDimensionAndLargestAgentFunctor
      : public Functor<void, Agent*, AgentHandle> {
    using Type = std::vector<std::array<double, 8>>;

    SimDimensionAndLargestAgentFunctor(Type& xmin, Type& xmax, Type& ymin,
                                       Type& ymax, Type& zmin, Type& zmax,
                                       Type& largest)
        : xmin_(xmin),
          xmax_(xmax),
          ymin_(ymin),
          ymax_(ymax),
          zmin_(zmin),
          zmax_(zmax),
          largest_(largest) {}

    void operator()(Agent* agent, AgentHandle) override {
      auto const* ti = ThreadInfo::GetInstance();
      auto tid = ti->GetMyThreadId();
      const auto& position = agent->GetPosition();
      // x
      if (position[0] < xmin_[tid][0]) {
        xmin_[tid][0] = position[0];
      }
      if (position[0] > xmax_[tid][0]) {
        xmax_[tid][0] = position[0];
      }
      // y
      if (position[1] < ymin_[tid][0]) {
        ymin_[tid][0] = position[1];
      }
      if (position[1] > ymax_[tid][0]) {
        ymax_[tid][0] = position[1];
      }
      // z
      if (position[2] < zmin_[tid][0]) {
        zmin_[tid][0] = position[2];
      }
      if (position[2] > zmax_[tid][0]) {
        zmax_[tid][0] = position[2];
      }
      // largest object
      auto diameter = agent->GetDiameter();
      if (diameter > largest_[tid][0]) {
        largest_[tid][0] = diameter;
      }
    }

    Type& xmin_;
    Type& xmax_;
    Type& ymin_;
    Type& ymax_;
    Type& zmin_;
    Type& zmax_;

    Type& largest_;
  };

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// agents
  void CalcSimDimensionsAndLargestAgent(
      std::array<double, 6>* ret_grid_dimensions) {
    auto* rm = Simulation::GetActive()->GetResourceManager();
    auto const* ti = ThreadInfo::GetInstance();
    const auto max_threads = ti->GetMaxThreads();
    // allocate version for each thread - avoid false sharing by padding them
    // assumes 64 byte cache lines (8 * sizeof(double))
    std::vector<std::array<double, 8>> xmin(max_threads, {{Math::kInfinity}});
    std::vector<std::array<double, 8>> xmax(max_threads, {{-Math::kInfinity}});

    std::vector<std::array<double, 8>> ymin(max_threads, {{Math::kInfinity}});
    std::vector<std::array<double, 8>> ymax(max_threads, {{-Math::kInfinity}});

    std::vector<std::array<double, 8>> zmin(max_threads, {{Math::kInfinity}});
    std::vector<std::array<double, 8>> zmax(max_threads, {{-Math::kInfinity}});

    std::vector<std::array<double, 8>> largest(max_threads, {{0}});

    SimDimensionAndLargestAgentFunctor functor(xmin, xmax, ymin, ymax, zmin,
                                               zmax, largest);
    rm->ForEachAgentParallel(1000, functor);

    // reduce partial results into global one
    double& gxmin = (*ret_grid_dimensions)[0];
    double& gxmax = (*ret_grid_dimensions)[1];
    double& gymin = (*ret_grid_dimensions)[2];
    double& gymax = (*ret_grid_dimensions)[3];
    double& gzmin = (*ret_grid_dimensions)[4];
    double& gzmax = (*ret_grid_dimensions)[5];
    for (int tid = 0; tid < max_threads; tid++) {
      // x
      if (xmin[tid][0] < gxmin) {
        gxmin = xmin[tid][0];
      }
      if (xmax[tid][0] > gxmax) {
        gxmax = xmax[tid][0];
      }
      // y
      if (ymin[tid][0] < gymin) {
        gymin = ymin[tid][0];
      }
      if (ymax[tid][0] > gymax) {
        gymax = ymax[tid][0];
      }
      // z
      if (zmin[tid][0] < gzmin) {
        gzmin = zmin[tid][0];
      }
      if (zmax[tid][0] > gzmax) {
        gzmax = zmax[tid][0];
      }
      // largest object
      if (largest[tid][0] > largest_object_size_) {
        largest_object_size_ = largest[tid][0];
      }
    }

    largest_object_size_squared_ = largest_object_size_ * largest_object_size_;
  }
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_ENVIRONMENT_H_
