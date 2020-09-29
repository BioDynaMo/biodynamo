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

#ifndef CORE_RESOURCE_MANAGER_H_
#define CORE_RESOURCE_MANAGER_H_

#include <omp.h>
#include <sched.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
#ifdef __APPLE__
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include "cl2.hpp"
#else
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl2.hpp>
#endif
#endif

#include "core/container/so_uid_map.h"
#include "core/diffusion_grid.h"
#include "core/operation/operation.h"
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_handle.h"
#include "core/sim_object/so_uid.h"
#include "core/sim_object/so_uid_generator.h"
#include "core/simulation.h"
#include "core/type_index.h"
#include "core/util/numa.h"
#include "core/util/root.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

/// ResourceManager stores simulation objects and diffusion grids and provides
/// methods to add, remove, and access them. Sim objects are uniquely identified
/// by their SoUid, and SoHandle. A SoHandle might change during the simulation.
class ResourceManager {
 public:
  explicit ResourceManager(TRootIOCtor* r) {}

  ResourceManager();

  virtual ~ResourceManager() {
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
    for (auto& numa_sos : sim_objects_) {
      for (auto* so : numa_sos) {
        delete so;
      }
    }
    if (type_index_) {
      delete type_index_;
    }
  }

  ResourceManager& operator=(ResourceManager&& other) {
    if (sim_objects_.size() != other.sim_objects_.size()) {
      Log::Fatal(
          "Restored ResourceManager has different number of NUMA nodes.");
    }
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
    for (auto& numa_sos : sim_objects_) {
      for (auto* so : numa_sos) {
        delete so;
      }
    }
    sim_objects_ = std::move(other.sim_objects_);
    diffusion_grids_ = std::move(other.diffusion_grids_);

    RestoreUidSoMap();
    // restore type_index_
    if (type_index_) {
      for (auto& numa_sos : sim_objects_) {
        for (auto* so : numa_sos) {
          type_index_->Add(so);
        }
      }
    }
    return *this;
  }

  void RestoreUidSoMap() {
    // rebuild uid_soh_map_
    uid_soh_map_.clear();
    auto* so_uid_generator = Simulation::GetActive()->GetSoUidGenerator();
    uid_soh_map_.resize(so_uid_generator->GetHighestIndex() + 1);
    for (unsigned n = 0; n < sim_objects_.size(); ++n) {
      for (unsigned i = 0; i < sim_objects_[n].size(); ++i) {
        auto* so = sim_objects_[n][i];
        this->uid_soh_map_.Insert(so->GetUid(), SoHandle(n, i));
      }
    }
  }

  SimObject* GetSimObject(const SoUid& uid) {
    if (!uid_soh_map_.Contains(uid)) {
      return nullptr;
    }
    auto& soh = uid_soh_map_[uid];
    return sim_objects_[soh.GetNumaNode()][soh.GetElementIdx()];
  }

  SimObject* GetSimObjectWithSoHandle(SoHandle soh) {
    return sim_objects_[soh.GetNumaNode()][soh.GetElementIdx()];
  }

  SoHandle GetSoHandle(const SoUid& uid) { return uid_soh_map_[uid]; }

  void AddDiffusionGrid(DiffusionGrid* dgrid) {
    uint64_t substance_id = dgrid->GetSubstanceId();
    auto search = diffusion_grids_.find(substance_id);
    if (search != diffusion_grids_.end()) {
      Log::Fatal("ResourceManager::AddDiffusionGrid",
                 "You tried to add a diffusion grid with an already existing "
                 "substance id. Please choose a different substance id.");
    } else {
      diffusion_grids_[substance_id] = dgrid;
    }
  }

  void RemoveDiffusionGrid(size_t substance_id) {
    auto search = diffusion_grids_.find(substance_id);
    if (search != diffusion_grids_.end()) {
      delete search->second;
      diffusion_grids_.erase(search);
    } else {
      Log::Fatal("ResourceManager::AddDiffusionGrid",
                 "You tried to remove a diffusion grid that does not exist.");
    }
  }

  /// Return the diffusion grid which holds the substance of specified id
  DiffusionGrid* GetDiffusionGrid(size_t substance_id) const {
    assert(substance_id < diffusion_grids_.size() &&
           "You tried to access a diffusion grid that does not exist!");
    return diffusion_grids_.at(substance_id);
  }

  /// Return the diffusion grid which holds the substance of specified name
  /// Caution: using this function in a tight loop will result in a slow
  /// simulation. Use `GetDiffusionGrid(size_t)` in those cases.
  DiffusionGrid* GetDiffusionGrid(std::string substance_name) const {
    for (auto& el : diffusion_grids_) {
      auto& dg = el.second;
      if (dg->GetSubstanceName() == substance_name) {
        return dg;
      }
    }
    assert(false &&
           "You tried to access a diffusion grid that does not exist! "
           "Did you specify the correct substance name?");
    return nullptr;
  }

  /// Execute the given functor for all diffusion grids
  ///     rm->ApplyOnAllDiffusionGrids([](DiffusionGrid* dgrid) {
  ///       ...
  ///     });
  template <typename TFunctor>
  void ApplyOnAllDiffusionGrids(TFunctor&& f) const {
    for (auto& el : diffusion_grids_) {
      f(el.second);
    }
  }

  /// Returns the total number of simulation objects if numa_node == -1
  /// Otherwise the number of sim_objects in the specific numa node
  size_t GetNumSimObjects(int numa_node = -1) const {
    if (numa_node == -1) {
      size_t num_so = 0;
      for (auto& numa_sos : sim_objects_) {
        num_so += numa_sos.size();
      }
      return num_so;
    } else {
      return sim_objects_[numa_node].size();
    }
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements([](SimObject* element) {
  ///                              std::cout << *element << std::endl;
  ///                          });
  virtual void ApplyOnAllElements(
      const std::function<void(SimObject*)>& function) {
    for (auto& numa_sos : sim_objects_) {
      for (auto* so : numa_sos) {
        function(so);
      }
    }
  }

  virtual void ApplyOnAllElements(
      const std::function<void(SimObject*, SoHandle)>& function) {
    for (uint64_t n = 0; n < sim_objects_.size(); ++n) {
      auto& numa_sos = sim_objects_[n];
      for (uint64_t i = 0; i < numa_sos.size(); ++i) {
        function(numa_sos[i], SoHandle(n, i));
      }
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ApplyOnAllElements
  virtual void ApplyOnAllElementsParallel(Functor<void, SimObject*>& function);

  /// Apply an operation on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ApplyOnAllElements
  virtual void ApplyOnAllElementsParallel(Operation& op);

  virtual void ApplyOnAllElementsParallel(
      Functor<void, SimObject*, SoHandle>& function);

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses dynamic scheduling and work stealing. Batch size controlled by
  /// `chunk`.
  /// \param chunk number of sim objects that are assigned to a thread (batch
  /// size)
  /// \see ApplyOnAllElements
  virtual void ApplyOnAllElementsParallelDynamic(
      uint64_t chunk, Functor<void, SimObject*, SoHandle>& function);

  /// Reserves enough memory to hold `capacity` number of simulation objects for
  /// each numa domain.
  void Reserve(size_t capacity) {
    for (auto& numa_sos : sim_objects_) {
      numa_sos.reserve(capacity);
    }
    if (type_index_) {
      type_index_->Reserve(capacity);
    }
  }

  /// Resize `sim_objects_[numa_node]` such that it holds `current + additional`
  /// elements after this call.
  /// Returns the size after
  uint64_t GrowSoContainer(size_t additional, size_t numa_node) {
    if (additional == 0) {
      return sim_objects_[numa_node].size();
    }
    auto current = sim_objects_[numa_node].size();
    if (current + additional < sim_objects_[numa_node].size()) {
      sim_objects_[numa_node].reserve((current + additional) * 1.5);
    }
    sim_objects_[numa_node].resize(current + additional);
    return current;
  }

  /// Returns true if a sim object with the given uid is stored in this
  /// ResourceManager.
  bool Contains(const SoUid& uid) const { return uid_soh_map_.Contains(uid); }

  /// Remove all simulation objects
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Clear() {
    uid_soh_map_.clear();
    for (auto& numa_sos : sim_objects_) {
      for (auto* so : numa_sos) {
        delete so;
      }
      numa_sos.clear();
    }
    if (type_index_) {
      type_index_->Clear();
    }
  }

  /// Reorder simulation objects such that, sim objects are distributed to NUMA
  /// nodes. Nearby sim objects will be moved to the same NUMA node.
  virtual void SortAndBalanceNumaNodes();

  void DebugNuma() const;

  /// NB: This method is not thread-safe! This function might invalidate
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void push_back(SimObject* so,  // NOLINT
                 typename SoHandle::NumaNode_t numa_node = 0) {
    auto uid = so->GetUid();
    if (uid.GetIndex() >= uid_soh_map_.size()) {
      uid_soh_map_.resize(uid.GetIndex() + 1);
    }
    sim_objects_[numa_node].push_back(so);
    uid_soh_map_.Insert(
        uid, SoHandle(numa_node, sim_objects_[numa_node].size() - 1));
    if (type_index_) {
      type_index_->Add(so);
    }
  }

  void ResizeUidSohMap() {
    auto* so_uid_generator = Simulation::GetActive()->GetSoUidGenerator();
    auto highest_idx = so_uid_generator->GetHighestIndex();
    auto new_size = highest_idx * 1.5 + 1;
    if (highest_idx >= uid_soh_map_.size()) {
      uid_soh_map_.resize(new_size);
    }
    if (type_index_) {
      type_index_->Reserve(new_size);
    }
  }

  void EndOfIteration() {
    // Check if SoUiD defragmentation should be turned on or off
    double utilization = static_cast<double>(uid_soh_map_.size()) /
                         static_cast<double>(GetNumSimObjects());
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();
    if (utilization < param->souid_defragmentation_low_watermark_) {
      sim->GetSoUidGenerator()->EnableDefragmentation(&uid_soh_map_);
    } else if (utilization > param->souid_defragmentation_high_watermark_) {
      sim->GetSoUidGenerator()->DisableDefragmentation();
    }
  }

  /// Adds `new_sim_objects` to `sim_objects_[numa_node]`. `offset` specifies
  /// the index at which the first element is inserted. Sim objects are inserted
  /// consecutively. This methos is thread safe only if insertion intervals do
  /// not overlap!
  virtual void AddNewSimObjects(
      typename SoHandle::NumaNode_t numa_node, uint64_t offset,
      const std::vector<SimObject*>& new_sim_objects) {
    uint64_t i = 0;
    for (auto* so : new_sim_objects) {
      auto uid = so->GetUid();
      uid_soh_map_.Insert(uid, SoHandle(numa_node, offset + i));
      sim_objects_[numa_node][offset + i] = so;
      i++;
    }
    if (type_index_) {
#pragma omp critical
      for (auto* so : new_sim_objects) {
        type_index_->Add(so);
      }
    }
  }

  /// Removes the simulation object with the given uid.\n
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Remove(const SoUid& uid) {
    // remove from map
    if (uid_soh_map_.Contains(uid)) {
      auto soh = uid_soh_map_[uid];
      uid_soh_map_.Remove(uid);
      // remove from vector
      auto& numa_sos = sim_objects_[soh.GetNumaNode()];
      SimObject* so = nullptr;
      if (soh.GetElementIdx() == numa_sos.size() - 1) {
        so = numa_sos.back();
        numa_sos.pop_back();
      } else {
        // swap
        so = numa_sos[soh.GetElementIdx()];
        auto* reordered = numa_sos.back();
        numa_sos[soh.GetElementIdx()] = reordered;
        numa_sos.pop_back();
        uid_soh_map_.Insert(reordered->GetUid(), soh);
      }
      if (type_index_) {
        type_index_->Remove(so);
      }
      delete so;
    }
  }

  const TypeIndex* GetTypeIndex() const { return type_index_; }

 protected:
  /// Maps an SoUid to its storage location in `sim_objects_` \n
  SoUidMap<SoHandle> uid_soh_map_ = SoUidMap<SoHandle>(100u);  //!
  /// Pointer container for all simulation objects
  std::vector<std::vector<SimObject*>> sim_objects_;
  /// Maps a diffusion grid ID to the pointer to the diffusion grid
  std::unordered_map<uint64_t, DiffusionGrid*> diffusion_grids_;

  ThreadInfo* thread_info_ = ThreadInfo::GetInstance();  //!

  TypeIndex* type_index_ = nullptr;

  friend class SimulationBackup;
  friend std::ostream& operator<<(std::ostream& os, const ResourceManager& rm);
  BDM_CLASS_DEF_NV(ResourceManager, 1);
};

inline std::ostream& operator<<(std::ostream& os, const ResourceManager& rm) {
  os << "\033[1mSimulation objects per numa node\033[0m"
     << std::endl;
  uint64_t cnt = 0;
  for (auto& numa_sos : rm.sim_objects_) {
    os << "numa node " << cnt++ << " -> size: " << numa_sos.size() << std::endl;
  }
  return os;
}

}  // namespace bdm

#endif  // CORE_RESOURCE_MANAGER_H_
