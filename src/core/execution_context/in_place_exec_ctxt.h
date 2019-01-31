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

#ifndef CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
#define CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_

#include <memory>
#include <mutex>
#include <vector>

#include "core/container/fixed_size_vector.h"
#include "core/resource_manager.h"

namespace bdm {

/// This execution context updates simulation objects in place. \n
/// Let's assume we have two sim objects `A, B` in our simulation that we want
/// to update to the next timestep `A*, B*`. If we have one thread it will first
/// update `A` and afterwards `B` and write the updates directly to the same
/// data structure. Therefore, before we start updating `B` the array looks
/// like this: `A*, B`. `B` already observes the updated `A`. \n
/// Operations in method `Execute` are executed in order given by the user.
/// Subsequent operations observe the changes of earlier operations.\n
/// In-place updates can lead to race conditions if simulation objects not only
/// modify themselves, but also neighbors. Therefore, a protection mechanism has
/// been added. If neighbors are not modified, this protection can be turned off
///  to improve performance using `DisableNeighborGuard()`. By default it is
/// turned on.\n
/// New sim objects will only be visible at the next iteration. \n
/// Also removal of a sim object happens at the end of each iteration.
class InPlaceExecutionContext {
 public:
  InPlaceExecutionContext() {
    // FIXME this doesn't work: must hold all new elements for all sim_objects
    // processed by this thread.
    // reserve enough memory to hold all new objects during one iteration of
    // one sim object. If more objects would be created (using `New`),
    // references would become invalid.
    // Alternative: use container that doesn't migrate objects.
    new_sim_objects_.Reserve(10);
  }

  /// This function is called at the beginning of each iteration.
  /// This function is not thread-safe.
  void SetupIteration() {
    // first iteration might have uncommited changes
    TearDownIteration();
  }

  /// This function is called at the end of each iteration. \n
  /// This function is not thread-safe. \n
  /// NB: Invalidates references and pointers to simulation objects.
  void TearDownIteration() {
    // new sim objects
    auto* rm = Simulation::GetActive()->GetResourceManager();
    new_sim_objects_.ApplyOnAllElements(
        [&](auto&& sim_object) { rm->push_back(&sim_object); });
    new_sim_objects_.Clear();

    // removed sim objects
    // remove them after adding new ones (maybe one has been removed
    // that was in new_sim_objects_)
    for (auto& uid : remove_) {
      rm->Remove(uid);
    }
    remove_.clear();
  }

  /// Execute a series of operations on a simulation object in the order given
  /// in the argument
  template <typename TFirstOp, typename... TOps>
  void Execute(SimObject& so, TFirstOp first_op, TOps... other_ops) {
    auto* grid = Simulation::GetActive()->GetGrid();
    auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
    if (nb_mutex_builder != nullptr) {
      auto mutex = nb_mutex_builder->GetMutex(so.GetBoxIdx());
      std::lock_guard<decltype(mutex)> guard(mutex);
      ExecuteInternal(so, first_op, other_ops...);
    } else {
      ExecuteInternal(so, first_op, other_ops...);
    }
  }

  void push_back(SimObject* new_so) {
    new_sim_objects_.push_back(new_so);
  }

  /// Forwards the call to `Grid::ForEachNeighborWithinRadius`
  /// Could be used to cache the results.
  template <typename TLambda>
  void ForEachNeighborWithinRadius(const TLambda& lambda, const SimObject& query,
                                   double squared_radius) {
    auto* grid = Simulation::GetActive()->GetGrid();
    return grid->ForEachNeighborWithinRadius(lambda, query, squared_radius);
  }

  SimObject* GetSimObject(SoUid uid) {
    if (new_sim_objects_.Contains(uid)) {
      return new_sim_objects_.template GetSimObject<TSo>(uid);
    } else {
      auto* rm = Simulation::GetActive()->GetResourceManager();
      return rm->GetSimObject(uid);
    }
  }

  const SimObject* GetConstSimObject(SoUid uid) {
    return GetSimObject(uid);
  }

  void RemoveFromSimulation(SoUid uid) {
    remove_.push_back(uid);
  }

  /// If a sim objects modifies other simulation objects while it is updated,
  /// race conditions can occur using this execution context. This function
  /// turns the protection mechanism off to improve performance. This is safe
  /// simulation objects only update themselves.
  void DisableNeighborGuard() {
    Simulation::GetActive()->GetGrid()->DisableNeighborMutexes();
  }

 private:
  /// Contains unique ids of sim objects that will be removed at the end of each
  /// iteration.
  std::vector<SoUid> remove_;

  /// Use seperate ResourceManager to store new objects, before they are added
  /// to the main ResourceManager. Using a ResourceManager adds
  /// some memory overhead, but avoids code duplication.
  ResourceManager new_sim_objects_;

  /// Execute a single operation on a simulation object
  template <typename TFirstOp>
  void ExecuteInternal(SimObject& so, TFirstOp first_op) {
    first_op(so);
  }

  /// Execute a series of operations on a simulation object in the order given
  /// in the argument
  template <typename TFirstOp, typename... TOps>
  void ExecuteInternal(SimObject& so, TFirstOp first_op, TOps... other_ops) {
    first_op(so);
    ExecuteInternal(so, other_ops...);
  }
};

}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
