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

#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <Rtypes.h>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// #ifdef USE_OPENCL
// #define __CL_ENABLE_EXCEPTIONS
// #ifdef __APPLE__
// #include <OpenCL/cl.hpp>
// #else
// #include <CL/cl.hpp>
// #endif
// #endif

#include "diffusion_grid.h"
// #include "simulation.h"
#include "simulation_object.h"
#include "transactional_vector.h"

namespace bdm {

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object.
/// The id is split into two parts: Type index and element index.
/// The first one is used to obtain the container in the ResourceManager, the
/// second specifies the element within this vector.
// FIXME
using SoHandle = uint64_t;

constexpr SoHandle kNullSoHandle = 0;

/// ResourceManager holds a container for each atomic type in the simulation.
/// It provides methods to get a certain container, execute a function on a
/// a certain element, all elements of a certain type or all elements inside
/// the ResourceManager. Elements are uniquely identified with its SoHandle.
/// Furthermore, the types specified in AtomicTypes are backend invariant
/// Hence it doesn't matter which version of the Backend is specified.
/// ResourceManager internally uses the TBackendWrapper parameter to convert
/// all atomic types to the desired backend.
/// This makes user code easier since atomic types can be specified as scalars.
/// @tparam TCompileTimeParam type that containes the compile time parameter for
/// a specific simulation. ResourceManager extracts Backend and AtomicTypes.
class ResourceManager {
 public:

  explicit ResourceManager(TRootIOCtor* r) {}

  /// Default constructor. Unfortunately needs to be public although it is
  /// a singleton to be able to use ROOT I/O
  ResourceManager() {
    // Soa container contain one element upon construction
    Clear();
  }

  /// Free the memory that was reserved for the diffusion grids
  virtual ~ResourceManager() {
    // for (auto* grid : diffusion_grids_) {
      // delete grid;
    // }
  }

  ResourceManager& operator=(ResourceManager&& other) {
    data_ = std::move(other.data_);
    // diffusion_grids_ = std::move(other.diffusion_grids_);
    return *this;
  }

  /// Return the container of diffusion grids
  std::vector<DiffusionGrid*>& GetDiffusionGrids() { return diffusion_grids_; }

  /// Return the diffusion grid which holds the substance of specified id
  DiffusionGrid* GetDiffusionGrid(size_t substance_id) {
    assert(substance_id < diffusion_grids_.size() &&
           "You tried to access a diffusion grid that does not exist!");
    return diffusion_grids_[substance_id];
  }

  /// Return the diffusion grid which holds the substance of specified name
  /// Caution: using this function in a tight loop will result in a slow
  /// simulation. Use `GetDiffusionGrid(size_t)` in those cases.
  DiffusionGrid* GetDiffusionGrid(std::string substance_name) {
    for (auto dg : diffusion_grids_) {
      if (dg->GetSubstanceName() == substance_name) {
        return dg;
      }
    }
    assert(false &&
           "You tried to access a diffusion grid that does not exist! "
           "Did you specify the correct substance name?");
    return nullptr;
  }

  /// Returns the total number of simulation objects
  size_t GetNumSimObjects() {
    return data_.size();
  }

  /// Apply a function on a certain element
  /// @param handle - simulation object id; specifies the tuple index and
  /// element index \see SoHandle
  /// @param function that will be called with the element as a parameter
  ///
  ///     rm->ApplyOnElement(handle, [](auto& element) {
  ///                          std::cout << element << std::endl;
  ///                       });
  template <typename TFunction>
  auto ApplyOnElement(SoHandle handle, TFunction&& function) {
    return function(data_[handle]);
  }

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, uint16_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  // template <typename TFunction>
  // void ApplyOnAllTypes(TFunction&& function) {
  //   // runtime dispatch - TODO(lukas) replace with c++17 std::apply
  //   for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
  //     ::bdm::Apply(&data_, i, [&](auto* container) { function(container, i); });
  //   }
  // }

  /// Apply a function on all container types. Function invocations are
  /// parallelized
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, uint16_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  // template <typename TFunction>
  // void ApplyOnAllTypesParallel(TFunction&& function) {
  //   // runtime dispatch - TODO(lukas) replace with c++17 std::apply
  //   for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
  //     ::bdm::Apply(&data_, i, [&](auto* container) { function(container, i); });
  //   }
  // }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements([](auto& element, SoHandle handle) {
  ///                              std::cout << element << std::endl;
  ///                          });
  template <typename TFunction>
  void ApplyOnAllElements(TFunction&& function) {
    for (auto* element : data_) {
      function(element);
    }
  }

  /// Apply a function on all elements in every container
  /// Function invocations are parallelized
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallel(TFunction&& function) {
#pragma omp parallel for
        for(uint64_t i = 0; i < data_.size(); i++) {
          function(data_[i]);
        }
  }

  /// Remove elements from each type
  void Clear() {
    data_.clear();
  }

  void push_back(SimulationObject* so) {  // NOLINT
    data_.push_back(so);
  }

// #ifdef USE_OPENCL
//   cl::Context* GetOpenCLContext() { return &opencl_context_; }
//   cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }
//   std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }
//   std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }
// #endif

  /// Create a new simulation object and return a reference to it.
  /// @tparam TScalarSo simulation object type with scalar backend
  /// @param args arguments which will be forwarded to the TScalarSo constructor
  /// @remarks Note that this function is not thread safe.
  template <typename TSo, typename... Args>
  TSo* New(Args... args) {
    auto idx = data_.DelayedPushBack(TSo(std::forward<Args>(args)...));
    return data_[idx];
  }

 private:
  /// creates one container for each type in Types.
  /// Container type is determined based on the specified Backend
  TransactionalVector<SimulationObject*> data_;
  std::vector<DiffusionGrid*> diffusion_grids_;

// #ifdef USE_OPENCL
//   cl::Context opencl_context_;             //!
//   cl::CommandQueue opencl_command_queue_;  //!
//   // Currently only support for one GPU device
//   std::vector<cl::Device> opencl_devices_;    //!
//   std::vector<cl::Program> opencl_programs_;  //!
// #endif

  // friend class SimulationBackup;
  // ClassDefNV(ResourceManager, 1);
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
