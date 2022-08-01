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

#include <omp.h>
#include <cassert>
#include <mutex>
#include <vector>
#include "core/agent/agent.h"
#include "core/container/math_array.h"
#include "core/functor.h"
#include "core/load_balance_info.h"
#include "core/resource_manager.h"

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
    assert(!omp_in_parallel() && "Update called in parallel region.");
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
  virtual void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                               const Agent& query, real_t squared_radius) = 0;

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
  virtual void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                               const Real3& query_position,
                               real_t squared_radius,
                               const Agent* query_agent = nullptr) = 0;

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

 protected:
  /// Member function that is called by Update() and ForcedUpdate(). Pure
  /// virtual.
  virtual void UpdateImplementation() = 0;
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_ENVIRONMENT_H_
