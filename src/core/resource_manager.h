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
#include <future>
#include <limits>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef USE_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif

#include "core/diffusion_grid.h"
#include "core/sim_object/so_uid.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/numa.h"
#include "core/util/root.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

/// ResourceManager holds a container for each atomic type in the simulation.
/// It provides methods to get a certain container, execute a function on a
/// a certain element, all elements of a certain type or all elements inside
/// the ResourceManager. Elements are uniquely identified with its SoHandle.
/// Furthermore, the types specified in SimObjectTypes are backend invariant
/// Hence it doesn't matter which version of the Backend is specified.
/// ResourceManager internally uses the TBackendWrapper parameter to convert
/// all SimObjectTypes to the desired backend.
/// This makes user code easier since SimObjectTypes can be specified as
/// scalars.
/// @tparam TCompileTimeParam type that containes the compile time parameter for
/// a specific simulation. ResourceManager extracts Backend and SimObjectTypes.
class ResourceManager {
 public:
  explicit ResourceManager(TRootIOCtor* r) {}

  /// Default constructor. Unfortunately needs to be public although it is
  /// a singleton to be able to use ROOT I/O
  ResourceManager() {}

  /// Free the memory that was reserved for the diffusion grids
  virtual ~ResourceManager() {
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
    for (auto& el : sim_objects_) {
      delete el.second;
    }
  }

  ResourceManager& operator=(ResourceManager&& other) {
    sim_objects_ = std::move(other.sim_objects_);
    sim_objects_ = std::move(other.sim_objects_);
    diffusion_grids_ = std::move(other.diffusion_grids_);
    return *this;
  }

  SimObject* GetSimObject(SoUid uid) {
    return sim_objects_[uid];
  }

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

  /// Returns the total number of simulation objects
  size_t GetNumSimObjects() const {
    return sim_objects_.size();
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements([](auto&& element) {
  ///                              std::cout << element << std::endl;
  ///                          });
  void ApplyOnAllElements(const std::function<void(SimObject*)>& function) {
    for(auto it = sim_objects_.begin(); it != sim_objects_.end(); ++it) {
      function(it->second);
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ApplyOnAllElements
  void ApplyOnAllElementsParallel(const std::function<void(SimObject*)>& function) {
    #pragma omp parallel
    for(auto it = sim_objects_.begin(); it != sim_objects_.end(); ++it) {
      function(it->second);
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses dynamic scheduling and work stealing. Batch size controlled by
  /// `chunk`.
  /// \param chunk number of sim objects that are assigned to a thread (batch
  /// size)
  /// \see ApplyOnAllElements
  void ApplyOnAllElementsParallelDynamic(uint64_t chunk, const std::function<void(SimObject*)>& function) {
    // #pragma omp parallel for schedule(dynamic, chunk)
    // FIXME
    for(auto it = sim_objects_.begin(); it != sim_objects_.end(); ++it) {
      function(it->second);
    }
  }

  /// Reserves enough memory to hold `capacity` number of simulation objects for
  /// each simulation object type.
  void Reserve(size_t capacity) {
    sim_objects_.reserve(capacity);
  }

  /// Returns true if a sim object with the given uid is stored in this
  /// ResourceManager.
  bool Contains(SoUid uid) const {
    return sim_objects_.find(uid) != sim_objects_.end();
  }

  /// Remove all simulation objects
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Clear() {
    sim_objects_.clear();
  }

  /// Reorder simulation objects such that, sim objects are distributed to NUMA
  /// nodes. Nearby sim objects will be moved to the same NUMA node.
  void SortAndBalanceNumaNodes() {
    // FIXME implement
  }

  /// NB: This method is not thread-safe! This function might invalidate
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void push_back(SimObject* so) {  // NOLINT
    sim_objects_[so->GetUid()] = so;
  }

  /// Removes the simulation object with the given uid.\n
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Remove(SoUid uid) {
    // remove from map
    auto it = this->sim_objects_.find(uid);
    if (it != this->sim_objects_.end()) {
      auto* so = it->second;
      delete so;
      sim_objects_.erase(it);
    }
  }

#ifdef USE_OPENCL
  cl::Context* GetOpenCLContext() { return &opencl_context_; }
  cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }
  std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }
  std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }
#endif

  /// Simulation object storgage. \n
  std::unordered_map<SoUid, SimObject*> sim_objects_;

  std::unordered_map<uint64_t, DiffusionGrid*> diffusion_grids_;

#ifdef USE_OPENCL
  cl::Context opencl_context_;             //!
  cl::CommandQueue opencl_command_queue_;  //!
  // Currently only support for one GPU device
  std::vector<cl::Device> opencl_devices_;    //!
  std::vector<cl::Program> opencl_programs_;  //!
#endif

  friend class SimulationBackup;
  BDM_CLASS_DEF_NV(ResourceManager, 1);
};

}  // namespace bdm

#endif  // CORE_RESOURCE_MANAGER_H_
