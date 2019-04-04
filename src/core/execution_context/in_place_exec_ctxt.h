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

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "core/container/fixed_size_vector.h"
#include "core/resource_manager.h"
#include "core/util/thread_info.h"
#include "core/util/timing.h"

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
template <typename TCTParam = CompileTimeParam<>>
class InPlaceExecutionContext {
 public:
  using Backend = typename TCTParam::SimulationBackend;
  using Types = typename TCTParam::SimObjectTypes;

  InPlaceExecutionContext() {
    // FIXME this doesn't work: must hold all new elements for all sim_objects
    // processed by this thread.
    // reserve enough memory to hold all new objects during one iteration of
    // one sim object. If more objects would be created (using `New`),
    // references would become invalid.
    // Alternative: use container that doesn't migrate objects.
    new_sim_objects_.Reserve(10);
  }

  /// This function is called at the beginning of each iteration to setup all
  /// execution contexts.
  /// This function is not thread-safe.
  /// NB: Invalidates references and pointers to simulation objects.
  void SetupIterationAll(
      const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {
    // first iteration might have uncommited changes
    TearDownIterationAll(all_exec_ctxts);
  }

  /// This function is called at the end of each iteration to tear down all
  /// execution contexts.
  /// This function is not thread-safe. \n
  /// NB: Invalidates references and pointers to simulation objects.
  template <typename TSimulation = Simulation<>>
  void TearDownIterationAll(
      const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {

    // std::cout << "\n\nTearDownIterationAll -----------------------------------" << std::endl;

    auto* rm = TSimulation::GetActive()->GetResourceManager();
    for (uint64_t t = 0; t < rm->NumberOfTypes(); ++t) {
      // group execution contexts by numa domain
      std::vector<uint64_t> new_so_per_numa(tinfo_->GetNumaNodes());
      std::vector<uint64_t> thread_offsets(tinfo_->GetMaxThreads());

      for (uint64_t tid = 0; tid < tinfo_->GetMaxThreads(); ++tid) {
        auto* ctxt = all_exec_ctxts[tid];
        int nid = tinfo_->GetNumaNode(tid);
        thread_offsets[tid] = new_so_per_numa[nid];
        new_so_per_numa[nid] += ctxt->new_sim_objects_.GetNumSimObjects(nid, t);
      }

      // reserve enough memory in ResourceManager
      std::vector<uint64_t> numa_offsets(tinfo_->GetNumaNodes());
      for (unsigned n = 0; n < new_so_per_numa.size(); n++) {
        numa_offsets[n] = rm->GrowSoContainer(new_so_per_numa[n], n, t);
      }

// add new_sim_objects_ to the ResourceManager in parallel
    // Timing timing("AddNewSimObjects");
#pragma omp parallel for schedule(static, 1)
      for (unsigned i = 0; i < all_exec_ctxts.size(); i++) {
        auto* ctxt = all_exec_ctxts[i];
        int nid = tinfo_->GetNumaNode(i);
        uint64_t offset = thread_offsets[i] + numa_offsets[nid];
        rm->AddNewSimObjects(nid, t, offset, ctxt->new_sim_objects_);
      }

      // // part 2 is not thread safe!
      // Timing timing("AddNewSimObjectsToSoStorageMap");
      // uint64_t tnso = 0;
      // for (unsigned i = 0; i < all_exec_ctxts.size(); i++) {
      //   auto* ctxt = all_exec_ctxts[i];
      //   int nid = tinfo_->GetNumaNode(i);
      //   uint64_t offset = thread_offsets[i] + numa_offsets[nid];
      //   tnso += ctxt->new_sim_objects_.GetNumSimObjects();
      //   rm->AddNewSimObjectsToSoStorageMap(nid, t, offset,
      //                                      ctxt->new_sim_objects_);
      // }
      // std::cout << "  nso " << tnso << std::endl;
    }

    // clear
    // Timing timing("AddNewSimObjectsToSoStorageMap");
// #pragma omp parallel for schedule(static, 1)
    for (unsigned i = 0; i < all_exec_ctxts.size(); i++) {
      auto* ctxt = all_exec_ctxts[i];
      rm->AddNewSimObjectsToSoStorageMap(ctxt->new_sim_objects_);
      ctxt->new_sim_objects_.Clear();
    }

    // remove
    for (unsigned i = 0; i < all_exec_ctxts.size(); i++) {
      auto* ctxt = all_exec_ctxts[i];
      // removed sim objects
      // remove them after adding new ones (maybe one has been removed
      // that was in new_sim_objects_)
      for (auto& uid : ctxt->remove_) {
        rm->Remove(uid);
      }
      ctxt->remove_.clear();
    }
  }

  /// Execute a series of operations on a simulation object in the order given
  /// in the argument
  template <typename TSo, typename TFirstOp, typename... TOps>
  void Execute(TSo&& so, TFirstOp first_op, TOps... other_ops) {
    auto* grid = Simulation<TCTParam>::GetActive()->GetGrid();
    auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
    if (nb_mutex_builder != nullptr) {
      auto nb_mutex = nb_mutex_builder->GetMutex(so.GetBoxIdx());
      std::lock_guard<decltype(nb_mutex)> guard(nb_mutex);
      neighbor_cache_.clear();
      ExecuteInternal(so, first_op, other_ops...);
    } else {
      neighbor_cache_.clear();
      ExecuteInternal(so, first_op, other_ops...);
    }
  }

  /// Create a new simulation object and return a reference to it.
  /// NB: A call to `New` might invalidate old references.
  /// @tparam TScalarSo simulation object type with scalar backend
  /// @param args arguments which will be forwarded to the TScalarSo constructor
  /// @remarks Note that this function is not thread safe.
  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Soa>::value,
                          typename TScalarSo::template Self<SoaRef>>::type
  New(Args... args) {
    TScalarSo so(std::forward<Args>(args)...);
    auto uid = so.GetUid();
    // #pragma omp critical
    // std::cout << "new so " << uid << " in tid" << omp_get_thread_num() << std::endl;
    std::lock_guard<AtomicMutex> guard(mutex_);
    new_sim_objects_.push_back(so);
    return new_sim_objects_.template GetSimObject<TScalarSo>(uid);
  }

  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Scalar>::value,
                          TScalarSo&>::type
  New(Args... args) {
    TScalarSo so(std::forward<Args>(args)...);
    auto uid = so.GetUid();
    // #pragma omp critical
    // std::cout << "new so " << uid << " in tid" << omp_get_thread_num() << std::endl;
    std::lock_guard<AtomicMutex> guard(mutex_);
    new_sim_objects_.push_back(so);
    return new_sim_objects_.template GetSimObject<TScalarSo>(uid);
  }

  /// Forwards the call to `Grid::ForEachNeighborWithinRadius`
  template <typename TLambda, typename TSo, typename TSimulation = Simulation<>>
  void ForEachNeighbor(const TLambda& lambda, const TSo& query) {
    // use values in cache
    if (neighbor_cache_.size() != 0) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      for (auto& pair : neighbor_cache_) {
        rm->ApplyOnElement(pair.first, [&](auto&& sim_object) {
          lambda(&sim_object, pair.second);
        });
      }
      return;
    }

    auto* grid = TSimulation::GetActive()->GetGrid();
    auto* param = TSimulation::GetActive()->GetParam();
    auto for_each = [&, this](auto* so, double squared_distance) {
      if (param->cache_neighbors_) {
        this->neighbor_cache_.push_back(
            std::make_pair(so->GetSoHandle(), squared_distance));
      }
      lambda(so, squared_distance);
    };

    grid->template ForEachNeighbor(for_each, query);
  }

  /// Forwards the call to `Grid::ForEachNeighborWithinRadius`
  template <typename TLambda, typename TSo, typename TSimulation = Simulation<>>
  void ForEachNeighborWithinRadius(const TLambda& lambda, const TSo& query,
                                   double squared_radius) {
    // use values in cache
    if (neighbor_cache_.size() != 0) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      for (auto& pair : neighbor_cache_) {
        if (pair.second < squared_radius) {
          rm->ApplyOnElement(pair.first,
                             [&](auto&& sim_object) { lambda(&sim_object); });
        }
      }
      return;
    }

    auto* grid = TSimulation::GetActive()->GetGrid();
    auto* param = TSimulation::GetActive()->GetParam();
    auto for_each = [&, this](auto* so, double squared_distance) {
      if (param->cache_neighbors_) {
        this->neighbor_cache_.push_back(
            std::make_pair(so->GetSoHandle(), squared_distance));
      }
      if (squared_distance < squared_radius) {
        lambda(so);
      }
    };

    grid->template ForEachNeighbor(for_each, query);
  }

  template <typename TSo, typename TSimBackend = Backend,
            typename TSimulation = Simulation<>>
  auto& GetSimObject(
      SoUid uid,
      typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type*
          ptr = 0) {
    auto soh = new_sim_objects_.GetSoHandle1(uid);
    if (soh != SoHandle()) {
      return new_sim_objects_.template GetSimObject<TSo>(soh);
    }

    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    soh = rm->GetSoHandle1(uid);
    if (soh != SoHandle()) {
      return rm->template GetSimObject<TSo>(soh);
    } else {
      // sim object must be cached in another InPlaceExecutionContext
      for (auto* ctxt : sim->GetAllExecCtxts()) {
        if (ctxt == this) {
          continue;
        }
        std::lock_guard<AtomicMutex> guard(ctxt->mutex_);
        auto soh = ctxt->new_sim_objects_.GetSoHandle1(uid);
        if (soh != SoHandle()) {
          return ctxt->new_sim_objects_.template GetSimObject<TSo>(soh);
        }
      }
    }
    Log::Fatal("GetSimObject", Concat("Could not find object with uid: ", uid));
  }

  template <typename TSo, typename TSimBackend = Backend,
            typename TSimulation = Simulation<>>
  auto GetSimObject(SoUid uid,
                    typename std::enable_if<
                        std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
    auto soh = new_sim_objects_.GetSoHandle1(uid);
    if (soh != SoHandle()) {
      return new_sim_objects_.template GetSimObject<TSo>(soh);
    }

    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    soh = rm->GetSoHandle1(uid);
    if (soh != SoHandle()) {
      return rm->template GetSimObject<TSo>(soh);
    } else {
      // sim object must be cached in another InPlaceExecutionContext
      for (auto* ctxt : sim->GetAllExecCtxts()) {
        if (ctxt == this) {
          continue;
        }
        std::lock_guard<AtomicMutex> guard(ctxt->mutex_);
        auto soh = ctxt->new_sim_objects_.GetSoHandle1(uid);
        if (soh != SoHandle()) {
          return ctxt->new_sim_objects_.template GetSimObject<TSo>(soh);
        }
      }
    }
    Log::Fatal("GetSimObject", Concat("Could not find object with uid: ", uid));
  }

  template <typename TSo, typename TSimBackend = Backend>
  const auto& GetConstSimObject(
      SoUid uid,
      typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type*
          ptr = 0) {
    return GetSimObject<TSo>(uid);
  }

  template <typename TSo, typename TSimBackend = Backend>
  const auto GetConstSimObject(
      SoUid uid,
      typename std::enable_if<std::is_same<TSimBackend, Soa>::value>::type*
          ptr = 0) {
    return GetSimObject<TSo>(uid);
  }

  void RemoveFromSimulation(SoUid uid) { remove_.push_back(uid); }

  /// If a sim objects modifies other simulation objects while it is updated,
  /// race conditions can occur using this execution context. This function
  /// turns the protection mechanism off to improve performance. This is safe
  /// simulation objects only update themselves.
  void DisableNeighborGuard() {
    Simulation<TCTParam>::GetActive()->GetGrid()->DisableNeighborMutexes();
  }

 private:
  class AtomicMutex {
   public:
    AtomicMutex() {}

    void lock() {  // NOLINT
      while (mutex_.test_and_set(std::memory_order_acquire)) {
      }
    }

    void unlock() {  // NOLINT
      mutex_.clear(std::memory_order_release);
    }

   private:
    std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;
  };

  ThreadInfo* tinfo_ = ThreadInfo::GetInstance();

  /// Contains unique ids of sim objects that will be removed at the end of each
  /// iteration.
  std::vector<SoUid> remove_;

  /// Use seperate ResourceManager to store new objects, before they are added
  /// to the main ResourceManager. Using a ResourceManager adds
  /// some memory overhead, but avoids code duplication.
  ResourceManager<TCTParam> new_sim_objects_;

  /// prevent race conditions for cached SimObjects
  AtomicMutex mutex_;

  std::vector<std::pair<SoHandle, double>> neighbor_cache_;

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
    ExecuteInternal(so, other_ops...);
  }
};

// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
