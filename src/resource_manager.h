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

#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include "root_util.h"

#ifdef USE_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif

#include "backend.h"
#include "compile_time_list.h"
#include "diffusion_grid.h"
#include "simulation.h"
#include "tuple_util.h"

namespace bdm {

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object.
/// The id is split into two parts: Type index and element index.
/// The first one is used to obtain the container in the ResourceManager, the
/// second specifies the element within this vector.
class SoHandle {
 public:
  constexpr SoHandle() noexcept
      : type_idx_(std::numeric_limits<decltype(type_idx_)>::max()),
        element_idx_(std::numeric_limits<decltype(element_idx_)>::max()) {}
  SoHandle(uint16_t type_idx, uint32_t element_idx)
      : type_idx_(type_idx), element_idx_(element_idx) {}
  uint16_t GetTypeIdx() const { return type_idx_; }
  uint32_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

  bool operator==(const SoHandle& other) const {
    return type_idx_ == other.type_idx_ && element_idx_ == other.element_idx_;
  }

  bool operator!=(const SoHandle& other) const { return !(*this == other); }

  bool operator<(const SoHandle& other) const {
    if (type_idx_ == other.type_idx_) {
      return element_idx_ < other.element_idx_;
    } else {
      return type_idx_ < other.type_idx_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const SoHandle& handle) {
    stream << "Type idx: " << handle.type_idx_
           << " element idx: " << handle.element_idx_;
    return stream;
  }

 private:
  // TODO(lukas) add using TypeIdx_t = uint16_t and
  // using ElementIdx_t = uint32_t
  uint16_t type_idx_;
  /// changed element index to uint32_t after issues with std::atomic with
  /// size 16 -> max element_idx: 4.294.967.296
  uint32_t element_idx_;

  BDM_CLASS_DEF_NV(SoHandle, 1);
};

constexpr SoHandle kNullSoHandle;

namespace detail {

/// \see bdm::ConvertToContainerTuple, CTList
template <typename Backend, typename... Types>
struct ConvertToContainerTuple {};

/// \see bdm::ConvertToContainerTuple, CTList
template <typename Backend, typename... Types>
struct ConvertToContainerTuple<Backend, CTList<Types...>> {
  // Helper alias to get the container type associated with Backend
  template <typename T>
  using Container = typename Backend::template Container<T>;
  // Helper type alias to get a type with certain Backend
  template <typename T>
  using ToBackend = typename T::template Self<Backend>;
  using type = std::tuple<Container<ToBackend<Types>>...>;  // NOLINT
};

/// Type trait to obtain the index of a type within a tuple.
/// Required to extract variadic types from withi a `CTList`
template <typename TSo, typename... Types>
struct ToIndex;

template <typename TSo, typename... Types>
struct ToIndex<TSo, CTList<Types...>> {
  static constexpr uint16_t value = GetIndex<TSo, Types...>();  // NOLINT
};

}  // namespace detail

/// Create a tuple of types in the parameter pack and wrap each type with
/// container.
/// @tparam Backend in which the variadic types should be stored in
/// @tparam TCTList type that wraps a CTList
/// which in turn contains the variadic template parameters
/// \see CTList
template <typename Backend, typename TCTList>
struct ConvertToContainerTuple {
  typedef typename detail::ConvertToContainerTuple<Backend, TCTList>::type
      type;  // NOLINT
};

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
template <typename TCompileTimeParam = CompileTimeParam<>>
class ResourceManager {
 public:
  using Backend = typename TCompileTimeParam::SimulationBackend;
  using Types = typename TCompileTimeParam::SimObjectTypes;
  /// Determine Container based on the Backend
  template <typename T>
  using TypeContainer = typename Backend::template Container<T>;
  /// Helper type alias to get a type with certain Backend
  template <typename T>
  using ToBackend = typename T::template Self<Backend>;

  /// Returns the number of simulation object types
  static constexpr size_t NumberOfTypes() {
    return std::tuple_size<decltype(data_)>::value;
  }

  template <typename TSo>
  static constexpr uint16_t GetTypeIndex() {
    using TSoScalar = typename TSo::template Self<Scalar>;
    return detail::ToIndex<TSoScalar, Types>::value;
  }

  explicit ResourceManager(TRootIOCtor* r) {}

  /// Default constructor. Unfortunately needs to be public although it is
  /// a singleton to be able to use ROOT I/O
  ResourceManager() {
    // Soa container contain one element upon construction
    Clear();
  }

  /// Free the memory that was reserved for the diffusion grids
  virtual ~ResourceManager() {
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
  }

  ResourceManager& operator=(ResourceManager&& other) {
    data_ = std::move(other.data_);
    diffusion_grids_ = std::move(other.diffusion_grids_);
    return *this;
  }

  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  TypeContainer<ToBackend<Type>>* Get() {
    return &std::get<TypeContainer<ToBackend<Type>>>(data_);
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
  size_t GetNumSimObjects() {
    size_t num_so = 0;
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i,
                   [&](auto* container) { num_so += container->size(); });
    }
    return num_so;
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
    auto type_idx = handle.GetTypeIdx();
    auto element_idx = handle.GetElementIdx();
    return ::bdm::Apply(&data_, type_idx, [&](auto* container) -> decltype(
                                              function((*container)[0])) {
      return function((*container)[element_idx]);
    });
  }

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, uint16_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypes(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) { function(container, i); });
    }
  }

  /// Apply a function on all container types. Function invocations are
  /// parallelized
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, uint16_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypesParallel(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) { function(container, i); });
    }
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllElements([](auto&& element, SoHandle handle) {
  ///                              std::cout << element << std::endl;
  ///                          });
  template <typename TFunction>
  void ApplyOnAllElements(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) {
        for (size_t e = 0; e < container->size(); e++) {
          function((*container)[e], SoHandle(i, e));
        }
      });
    }
  }

  /// Apply a function on all elements in every container
  /// Function invocations are parallelized
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallel(TFunction&& function) {
    // runtime dispatch - TODO(lukas) replace with c++17 std::apply
    for (uint16_t i = 0; i < std::tuple_size<decltype(data_)>::value; i++) {
      ::bdm::Apply(&data_, i, [&](auto* container) {
#pragma omp parallel for
        for (size_t e = 0; e < container->size(); e++) {
          function((*container)[e], SoHandle(i, e));
        }
      });
    }
  }

  /// Remove elements from each type
  void Clear() {
    ApplyOnAllTypes(
        [](auto* container, uint16_t type_idx) { container->clear(); });
  }

  template <typename TSo>
  void push_back(const TSo& so) {  // NOLINT
    Get<TSo>()->push_back(so);
  }

#ifdef USE_OPENCL
  cl::Context* GetOpenCLContext() { return &opencl_context_; }
  cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }
  std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }
  std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }
#endif

  /// Create a new simulation object and return a reference to it.
  /// @tparam TScalarSo simulation object type with scalar backend
  /// @param args arguments which will be forwarded to the TScalarSo constructor
  /// @remarks Note that this function is not thread safe.
  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Soa>::value,
                          typename TScalarSo::template Self<SoaRef>>::type
  New(Args... args) {
    auto container = Get<TScalarSo>();
    auto idx =
        container->DelayedPushBack(TScalarSo(std::forward<Args>(args)...));
    return (*container)[idx];
  }

  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Scalar>::value,
                          TScalarSo&>::type
  New(Args... args) {
    auto container = Get<TScalarSo>();
    auto idx =
        container->DelayedPushBack(TScalarSo(std::forward<Args>(args)...));
    return (*container)[idx];
  }

 private:
  /// creates one container for each type in Types.
  /// Container type is determined based on the specified Backend
  typename ConvertToContainerTuple<Backend, Types>::type data_;
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

#endif  // RESOURCE_MANAGER_H_
