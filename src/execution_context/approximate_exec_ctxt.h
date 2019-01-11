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

#ifndef EXECUTION_CONTEXT_APPROXIMATE_EXEC_CTXT_H_
#define EXECUTION_CONTEXT_APPROXIMATE_EXEC_CTXT_H_

#include <vector>
#include <memory>

#include "resource_manager.h"

namespace bdm {

/// This class ensures thread-safety for the ApproximateExecCtxt for the case
/// that a simulation object modifies its neighbors.
class Locks {
public:
  template <typename TSimulation = Simulation<>>
  void Setup() {
    auto* grid = TSimulation::GetActive()->GetGrid();
    mutexes_.resize(grid->GetNumBoxes());
    for (auto& mutex : mutexes_) {
      mutex = ATOMIC_FLAG_INIT;
    }
  }

  template <typename TSimulation = Simulation<>>
  std::lock_guard<NeighborhoodMutex> GetLockGuard(uint64_t box_idx) {
    auto* grid = TSimulation::GetActive()->GetGrid();
    return std::lock_guard<NeighborhoodMutex>(box_idx, grid->GetMooreBoxeIndices());
  }

  /// The NeighborhoodMutex class is a synchronization primitive that can be
  /// used to protect sim_objects data from being simultaneously accessed by
  /// multiple threads.
  class NeighborhoodMutex {
    public:
      NeighborhoodMutex(uint64_t box_idx, FixedSizeVector&& mutex_indices, Locks* locks) :
        box_idx_(box_idx), mutex_indices_(mutex_indices), locks_(locks) {
        // Deadlocks occur if mutliple threads try to acquire the same locks,
        // but in different order.
        // -> sort to avoid deadlocks - see lock ordering
        std::sort(mutex_indices_.begin(), mutex_indices_.end());
      }

      void lock() {
        for(auto idx : mutex_indices_) {
          auto& mutex = locks->mutexes_[idx];
          while (mutex.test_and_set(std::memory_order_acquire))  // acquire lock
             ; // spin
        }
      }

      void unlock() {
        for(auto idx : mutex_indices_) {
          auto& mutex = locks->mutexes_[idx];
          mutex.clear(std::memory_order_release);
        }
      }

    private:
      uint64_t box_idx_;
      FixedSizeVector<uint64_t, 27> mutex_indices_;
      Locks* locks_;
  };

 private:
  /// one mutex for each box in `Grid::boxes_`
  std::vector<std::atomic_flag> mutexes_;
};

// TODO rename to InPlaceExecCtxt ?
// TODO documentation + test
template <typename TCTParam = CompileTimeParam<>>
class ApproximateExecCtxt {
public:
  using Backend = typename TCTParam::SimulationBackend;
  using Types = typename TCTParam::SimObjectTypes;

  ApproximateExecCtxt() {
    // reserve enough memory to hold all new objects during one iteration of
    // one sim object. If more objects would be created (using `New`),
    // references would become invalid.
    // Alternative: use container that doesn't migrate objects.
    new_sim_objects_.Reserve(10);
  }

  void SetupIteration() {
    // first iteration might have uncommited changes
    TearDownIteration();

    if(locks_ != nullptr) {
      locks_->Setup();
    }
  }

  template <typename TSimulation = Simulation<>>
  void TearDownIteration() {
    // FIXME this is probably a critical section

    // new sim objects
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    new_sim_objects_.ApplyOnAllElements([&](auto&& sim_object, SoHandle){
      rm->push_back(sim_object);
    });
    new_sim_objects_.Clear();

    // removed sim objects
    // remove them after adding new ones (maybe one has been removed
    // that was in new_sim_objects_)
    for(auto& uid : remove_) {
      rm->Remove(uid);
    }
    remove_.clear();
  }

  /// Execute a series of operations on a simulation object in the order given
  /// in the argument
  template <typename TSo, typename TFirstOp, typename... TOps>
  void Execute(TSo&& so, TFirstOp first_op, TOps... other_ops) {
    if (locks_ != nullptr) {
      auto lock_guard = locks_->GetLockGuard(so.GetBoxIdx());
      ExecuteInternal(so, first_op, other_ops...);
    } else {
      ExecuteInternal(so, first_op, other_ops...);
    }
  }

  /// Create a new simulation object and return a reference to it.
  /// @tparam TScalarSo simulation object type with scalar backend
  /// @param args arguments which will be forwarded to the TScalarSo constructor
  /// @remarks Note that this function is not thread safe.
  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Soa>::value,
                          typename TScalarSo::template Self<SoaRef>>::type
  New(Args... args) {
    TScalarSo so(std::forward<Args>(args)...);
    auto uid = so.GetUid();
    new_sim_objects_.push_back(so);
    return new_sim_objects_.template GetSimObject<TScalarSo>(uid);
  }

  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Scalar>::value,
                          TScalarSo&>::type
  New(Args... args) {
    TScalarSo so(std::forward<Args>(args)...);
    auto uid = so.GetUid();
    new_sim_objects_.push_back(so);
    return new_sim_objects_.template GetSimObject<TScalarSo>(uid);
  }

  /// Forwards the call to `Grid::ForEachNeighborWithinRadius`
  /// Could be used to cache the results.
  template <typename TLambda, typename TSo, typename TSimulation = Simulation<>>
  void ForEachNeighborWithinRadius(const TLambda& lambda, const TSo& query,
                                   double squared_radius) {
     auto* grid = TSimulation::GetActive()->GetGrid();
     return grid->template ForEachNeighborWithinRadius(lambda, query, squared_radius);
  }

  template <typename TSo, typename TSimBackend = Backend, typename TSimulation = Simulation<>>
  auto&& GetSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type* ptr = 0) {
    // check if the uid corresponds to a new object not yet in the Rm
    if (new_sim_objects_.Contains(uid)) {
      return new_sim_objects_.template GetSimObject<TSo>(uid);
    } else {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      return rm->template GetSimObject<TSo>(uid);
    }
  }

  template <typename TSo, typename TSimBackend = Backend, typename TSimulation = Simulation<>>
  auto GetSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
    if (new_sim_objects_.Contains(uid)) {
      return new_sim_objects_.template GetSimObject<TSo>(uid);
    } else {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      return rm->template GetSimObject<TSo>(uid);
    }
  }

  template <typename TSo, typename TSimBackend = Backend, typename TSimulation = Simulation<>>
  const auto&& GetConstSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type* ptr = 0) {
    return GetSimObject<TSo>(uid);
  }

  template <typename TSo, typename TSimBackend = Backend, typename TSimulation = Simulation<>>
  const auto GetConstSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
    return GetSimObject<TSo>(uid);
  }

  template <typename TSimulation = Simulation<>>
  void RemoveFromSimulation(SoUid uid) {
    remove_.push_back(uid);
  }

  /// If a sim objects modifies other simulation objects while it is updated,
  /// race conditions can occur using this exection context. This function turns
  /// on the protection mechanism. The protection mechanism is turned off by
  /// default to avoid unnecessary overhead for simulations that do not require
  /// this feature.
  void ModifyOtherSimObjects() {
    if(locks_ != nullptr) {
      locks_ = std::make_unique<Locks>();
    }
  }

private:
  static std::unique_ptr<Locks> locks_;

  std::vector<SoUid> remove_;

  /// Use seperate ResourceManager to store new objects, before they are added
  /// to the main ResourceManager. Using a ResourceManager adds
  /// some memory overhead, but avoids code duplication.
  ResourceManager<TCTParam> new_sim_objects_;


  /// Execute a single operation on a simulation object
  template <typename TSo, typename TFirstOp>
  void ExecuteInternal(TSo&& so, TFirstOp first_op) {
    first_op(so);
  }

  /// Execute a series of operations on a simulation object in the order given
  /// in the argument
  template <typename TSo, typename TFirstOp, typename... TOps>
  void ExecuteInternal(TSo&& so, TFirstOp first_op, TOps... other_ops) {
    first_op(so);
    Execute(so, other_ops...);
  }
};

}  // namespace bdm

#endif  // EXECUTION_CONTEXT_APPROXIMATE_EXECUTION_CTXT_H_
