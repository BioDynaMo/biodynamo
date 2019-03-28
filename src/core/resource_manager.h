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
#include "core/param/compile_time_list.h"
#include "core/sim_object/backend.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/numa.h"
#include "core/util/root.h"
#include "core/util/thread_info.h"
#include "core/util/tuple.h"
#include "core/util/type.h"

namespace bdm {

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object. \n
/// Points to the storage location of a sim object inside ResourceManager.\n
/// The id is split into three parts: Numa node, type index and element index.
/// The first one is used to obtain the numa storage, the second the correct
/// container in the ResourceManager, and the third specifies the element within
/// this vector.
class SoHandle {
 public:
  using TypeIdx_t = uint16_t;
  using NumaNode_t = uint16_t;
  using ElementIdx_t = uint32_t;

  constexpr SoHandle() noexcept
      : numa_node_(std::numeric_limits<NumaNode_t>::max()),
        type_idx_(std::numeric_limits<TypeIdx_t>::max()),
        element_idx_(std::numeric_limits<ElementIdx_t>::max()) {}

  SoHandle(TypeIdx_t type_idx, ElementIdx_t element_idx)
      : numa_node_(0), type_idx_(type_idx), element_idx_(element_idx) {}

  SoHandle(NumaNode_t numa_node, TypeIdx_t type_idx, ElementIdx_t element_idx)
      : numa_node_(numa_node), type_idx_(type_idx), element_idx_(element_idx) {}

  NumaNode_t GetNumaNode() const { return numa_node_; }
  TypeIdx_t GetTypeIdx() const { return type_idx_; }
  ElementIdx_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(ElementIdx_t element_idx) { element_idx_ = element_idx; }

  bool operator==(const SoHandle& other) const {
    return type_idx_ == other.type_idx_ && numa_node_ == other.numa_node_ &&
           element_idx_ == other.element_idx_;
  }

  bool operator!=(const SoHandle& other) const { return !(*this == other); }

  bool operator<(const SoHandle& other) const {
    if (type_idx_ == other.type_idx_ && numa_node_ == other.numa_node_) {
      return element_idx_ < other.element_idx_;
    } else if (type_idx_ == other.type_idx_) {
      return numa_node_ < other.numa_node_;
    } else {
      return type_idx_ < other.type_idx_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const SoHandle& handle) {
    stream << "Numa node: " << handle.numa_node_
           << " type idx: " << handle.type_idx_
           << " element idx: " << handle.element_idx_;
    return stream;
  }

 private:
  NumaNode_t numa_node_;

  TypeIdx_t type_idx_;

  /// changed element index to uint32_t after issues with std::atomic with
  /// size 16 -> max element_idx: 4.294.967.296
  ElementIdx_t element_idx_;

  BDM_CLASS_DEF_NV(SoHandle, 1);
};

constexpr SoHandle kNullSoHandle;

namespace detail {

/// \see bdm::ToTupleOfSOContainers, CTList
template <typename Backend, typename... Types>
struct ToTupleOfSOContainers {};

/// \see bdm::ToTupleOfSOContainers, CTList
template <typename Backend, typename... Types>
struct ToTupleOfSOContainers<Backend, CTList<Types...>> {
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
struct ToTupleOfSOContainers {
  typedef typename detail::ToTupleOfSOContainers<Backend, TCTList>::type
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
    return std::tuple_size<TupleOfSOContainers>::value;
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
    // Must be called prior any other function call to libnuma
    if (auto ret = numa_available() == -1) {
      Log::Fatal("ResourceManager",
                 "Call to numa_available failed with return code: ", ret);
    }
    // create a sim object storage instance for each numa node
    numa_nodes_ = numa_num_configured_nodes();
    sim_objects_ = new TupleOfSOContainers[numa_nodes_];
    // Soa container contain one element upon construction
    Clear();
  }

  /// Free the memory that was reserved for the diffusion grids
  virtual ~ResourceManager() {
    delete[] sim_objects_;
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
  }

  ResourceManager& operator=(ResourceManager&& other) {
    if (numa_nodes_ != other.numa_nodes_) {
      Log::Fatal(
          "Restored ResourceManager has different number of NUMA nodes.");
    }
    delete[] sim_objects_;
    sim_objects_ = other.sim_objects_;
    other.sim_objects_ = nullptr;
    numa_nodes_ = other.numa_nodes_;
    diffusion_grids_ = std::move(other.diffusion_grids_);

    RestoreUidSoMap();
    return *this;
  }

  void RestoreUidSoMap() {
    // rebuild so_storage_location_
    so_storage_location_.clear();
    ApplyOnAllElements([this](auto&& so, SoHandle handle) {
      this->so_storage_location_[so.GetUid()] = handle;
    });
  }

  SoHandle GetSoHandle(SoUid so_uid) const {
    return so_storage_location_.at(so_uid);
  }

  template <typename TSo, typename TSimBackend = Backend>
  auto&& GetSimObject(
      SoHandle handle,
      typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type*
          ptr = 0) {
    return (*GetContainer<TSo>(handle.GetNumaNode()))[handle.GetElementIdx()];
  }

  template <typename TSo, typename TSimBackend = Backend>
  auto GetSimObject(SoHandle handle,
                    typename std::enable_if<
                        std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
    return (*GetContainer<TSo>(handle.GetNumaNode()))[handle.GetElementIdx()];
  }

  template <typename TSo, typename TSimBackend = Backend>
  auto&& GetSimObject(
      SoUid uid,
      typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type*
          ptr = 0) {
    auto handle = so_storage_location_[uid];
    return (*GetContainer<TSo>(handle.GetNumaNode()))[handle.GetElementIdx()];
  }

  template <typename TSo, typename TSimBackend = Backend>
  auto GetSimObject(SoUid uid,
                    typename std::enable_if<
                        std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
    auto handle = so_storage_location_[uid];
    return (*GetContainer<TSo>(handle.GetNumaNode()))[handle.GetElementIdx()];
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

  uint16_t GetNumNumaNodes() { return numa_nodes_; }

  /// Returns the total number of simulation objects
  size_t GetNumSimObjects() {
    size_t num_so = 0;
    for (uint16_t n = 0; n < numa_nodes_; n++) {
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value;
           i++) {
        ::bdm::Apply(&sim_objects_[n], i,
                     [&](auto* container) { num_so += container->size(); });
      }
    }
    return num_so;
  }

  size_t GetNumSimObjects(uint16_t numa_node, uint16_t type_id) {
    size_t num_so = 0;
    ::bdm::Apply(&sim_objects_[numa_node], type_id,
                 [&](auto* container) { num_so = container->size(); });
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
    auto numa_node = handle.GetNumaNode();
    auto element_idx = handle.GetElementIdx();
    return ::bdm::Apply(
        &sim_objects_[numa_node], type_idx,
        [&](auto* container) -> decltype(function((*container)[0])) {
          return function((*container)[element_idx]);
        });
  }

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, typename SoHandle::NumaNode_t
  ///     numa_node, typename SoHandle::TypeIdx_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypes(TFunction&& function) {
    for (uint16_t n = 0; n < numa_nodes_; n++) {
      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value;
           i++) {
        ::bdm::Apply(&sim_objects_[n], i,
                     [&](auto* container) { function(container, n, i); });
      }
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
    for (uint16_t n = 0; n < numa_nodes_; n++) {
      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value;
           i++) {
        ::bdm::Apply(&sim_objects_[n], i, [&](auto* container) {
          for (size_t e = 0; e < container->size(); e++) {
            function((*container)[e], SoHandle(n, i, e));
          }
        });
      }
    }
  }

  /// Apply a function on all elements in every container
  /// Function invocations are parallelized
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallelNested(TFunction&& function) {
    auto nodes = numa_num_configured_nodes();
    auto cores = numa_num_configured_cpus() / 2;
    auto cores_per_node = cores / nodes;

    omp_set_nested(1);  // TODO(lukas) move to generatl openmp initialization

#pragma omp parallel num_threads(nodes)
    {
      auto numa_thread_id = omp_get_thread_num();
      numa_run_on_node(numa_thread_id);

      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value;
           i++) {
        ::bdm::Apply(&sim_objects_[numa_thread_id], i, [&](auto* container) {
#pragma omp parallel num_threads(cores_per_node)
          {
#pragma omp for
            for (size_t e = 0; e < container->size(); e++) {
              function((*container)[e], SoHandle(numa_thread_id, i, e));
            }
          }
        });
      }
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallel(TFunction&& function) {
    for (uint16_t t = 0; t < NumberOfTypes(); ++t) {
      // only needed to get the type of the container
      ::bdm::Apply(&sim_objects_[0], t, [&](auto* container) {
        // collect all containers of this type
        std::vector<decltype(container)> so_containers;
        so_containers.resize(numa_nodes_);
        so_containers[0] = container;
        for (uint16_t n = 1; n < numa_nodes_; n++) {
          ::bdm::Apply(&sim_objects_[n], t, [&](auto* container) {
            if (std::is_same<raw_type<decltype(so_containers[n])>,
                             raw_type<decltype(container)>>::value) {
              auto* tmp =
                  reinterpret_cast<raw_type<decltype(so_containers[n])>*>(
                      container);
              so_containers[n] = tmp;
            }
          });
        }

#pragma omp parallel
        {
          auto tid = omp_get_thread_num();
          auto nid = thread_info_->GetNumaNode(tid);
          auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
          auto* so_container = so_containers[nid];
          assert(thread_info_->GetNumaNode(tid) ==
                 numa_node_of_cpu(sched_getcpu()));

          // use static scheduling for now
          auto correction = so_container->size() % threads_in_numa == 0 ? 0 : 1;
          auto chunk = so_container->size() / threads_in_numa + correction;
          auto start = thread_info_->GetNumaThreadId(tid) * chunk;
          auto end = std::min(so_container->size(), start + chunk);

          for (uint64_t i = start; i < end; ++i) {
            function((*so_container)[i], SoHandle(nid, t, i));
          }
        }
      });
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses dynamic scheduling and work stealing. Batch size controlled by
  /// `chunk`.
  /// \param chunk number of sim objects that are assigned to a thread (batch
  /// size)
  /// \see ApplyOnAllElements
  template <typename TFunction>
  void ApplyOnAllElementsParallelDynamic(uint64_t chunk, TFunction&& function) {
    auto chunk_param = chunk;
    for (uint16_t t = 0; t < NumberOfTypes(); ++t) {
      // only needed to get the type of the container
      ::bdm::Apply(&sim_objects_[0], t, [&](auto* container) {
        // collect all containers of this type
        std::vector<decltype(container)> so_containers;
        so_containers.resize(numa_nodes_);
        so_containers[0] = container;
        uint64_t num_so = container->size();
        for (uint16_t n = 1; n < numa_nodes_; n++) {
          ::bdm::Apply(&sim_objects_[n], t, [&](auto* container) {
            if (std::is_same<raw_type<decltype(so_containers[n])>,
                             raw_type<decltype(container)>>::value) {
              auto* tmp =
                  reinterpret_cast<raw_type<decltype(so_containers[n])>*>(
                      container);
              so_containers[n] = tmp;
              num_so += tmp->size();
            }
          });
        }

        // adapt chunk size
        uint64_t factor =
            (num_so / thread_info_->GetMaxThreads()) / chunk_param;
        chunk = (num_so / thread_info_->GetMaxThreads()) / (factor + 1);
        chunk = chunk >= 1 ? chunk : 1;

        // use dynamic scheduling
        // Unfortunately openmp's built in functionality can't be used, since
        // threads belong to different numa domains and thus operate on
        // different containers
        auto max_threads = omp_get_max_threads();
        std::vector<uint64_t> num_chunks_per_numa(numa_nodes_);
        for (int n = 0; n < numa_nodes_; n++) {
          auto correction = so_containers[n]->size() % chunk == 0 ? 0 : 1;
          num_chunks_per_numa[n] =
              so_containers[n]->size() / chunk + correction;
        }

        std::vector<std::atomic<uint64_t>*> counters(max_threads, nullptr);
        std::vector<uint64_t> max_counters(max_threads);
        for (int thread_cnt = 0; thread_cnt < max_threads; thread_cnt++) {
          uint64_t current_nid = thread_info_->GetNumaNode(thread_cnt);

          auto correction =
              num_chunks_per_numa[current_nid] %
                          thread_info_->GetThreadsInNumaNode(current_nid) ==
                      0
                  ? 0
                  : 1;
          uint64_t num_chunks_per_thread =
              num_chunks_per_numa[current_nid] /
                  thread_info_->GetThreadsInNumaNode(current_nid) +
              correction;
          auto start =
              num_chunks_per_thread * thread_info_->GetNumaThreadId(thread_cnt);
          auto end = std::min(num_chunks_per_numa[current_nid],
                              start + num_chunks_per_thread);

          counters[thread_cnt] = new std::atomic<uint64_t>(start);
          max_counters[thread_cnt] = end;
        }

#pragma omp parallel
        {
          auto tid = omp_get_thread_num();
          auto nid = thread_info_->GetNumaNode(tid);
          // thread private variables (compilation error with
          // firstprivate(chunk, numa_node_) with some openmp versions clause)
          auto p_numa_nodes = numa_nodes_;
          auto p_max_threads = omp_get_max_threads();
          auto p_chunk = chunk;
          assert(thread_info_->GetNumaNode(tid) ==
                 numa_node_of_cpu(sched_getcpu()));

          // dynamic scheduling
          uint64_t start = 0;
          uint64_t end = 0;

          // this loop implements work stealing from other threads if there
          // are imbalances.
          for (int n = 0; n < p_numa_nodes; n++) {
            uint64_t current_nid = (nid + n) % p_numa_nodes;
            for (int thread_cnt = 0; thread_cnt < p_max_threads; thread_cnt++) {
              // uint64_t thread_cnt = 0;
              uint64_t current_tid = (tid + thread_cnt) % p_max_threads;
              if (current_nid != thread_info_->GetNumaNode(current_tid)) {
                continue;
              }

              auto* so_container = so_containers[current_nid];
              uint64_t old_count = (*(counters[current_tid]))++;
              while (old_count < max_counters[current_tid]) {
                start = old_count * p_chunk;
                end = std::min(static_cast<uint64_t>(so_container->size()),
                               start + p_chunk);

                for (uint64_t i = start; i < end; ++i) {
                  function((*so_container)[i], SoHandle(current_nid, t, i));
                }

                old_count = (*(counters[current_tid]))++;
              }
            }  // work stealing loop numa_nodes_
          }    // work stealing loop  threads
        }

        for (auto* counter : counters) {
          delete counter;
        }
      });
    }
  }

  template <typename TFunction>
  void ApplyOnAllElementsParallelDynamicAlternativeWS(uint64_t chunk,
                                                      TFunction&& function) {
    for (uint16_t t = 0; t < NumberOfTypes(); ++t) {
      // only needed to get the type of the container
      ::bdm::Apply(&sim_objects_[0], t, [&](auto* container) {
        // collect all containers of this type
        std::vector<decltype(container)> so_containers;
        so_containers.resize(numa_nodes_);
        so_containers[0] = container;
        for (uint16_t n = 1; n < numa_nodes_; n++) {
          ::bdm::Apply(&sim_objects_[n], t, [&](auto* container) {
            if (std::is_same<raw_type<decltype(so_containers[n])>,
                             raw_type<decltype(container)>>::value) {
              auto* tmp =
                  reinterpret_cast<raw_type<decltype(so_containers[n])>*>(
                      container);
              so_containers[n] = tmp;
            }
          });
        }

        // use dynamic scheduling
        // Unfortunately openmp's built in functionality can't be used, since
        // threads belong to different numa domains and thus operate on
        // different containers
        std::vector<std::atomic<uint64_t>*> counters(numa_nodes_, nullptr);
        std::vector<uint64_t> max_counters(numa_nodes_);
        for (int n = 0; n < numa_nodes_; n++) {
          counters[n] = new std::atomic<uint64_t>(0);
          // calculate value max_counters for each numa domain
          auto correction = so_containers[n]->size() % chunk == 0 ? 0 : 1;
          max_counters[n] = so_containers[n]->size() / chunk + correction;
        }

#pragma omp parallel
        {
          auto tid = omp_get_thread_num();
          auto nid = thread_info_->GetNumaNode(tid);
          // thread private variables (compilation error with
          // firstprivate(chunk, numa_node_) with some openmp versions clause)
          auto p_numa_nodes = numa_nodes_;
          auto p_chunk = chunk;
          assert(thread_info_->GetNumaNode(tid) ==
                 numa_node_of_cpu(sched_getcpu()));

          // dynamic scheduling
          uint64_t start = 0;
          uint64_t end = 0;

          uint64_t current_nid = nid;

          auto* so_container = so_containers[current_nid];
          uint64_t old_count = (*(counters[current_nid]))++;
          while (old_count <= max_counters[current_nid]) {
            start = old_count * p_chunk;
            end = std::min(static_cast<uint64_t>(so_container->size()),
                           start + p_chunk);

            for (uint64_t i = start; i < end; ++i) {
              function((*so_container)[i], SoHandle(current_nid, t, i));
            }

            old_count = (*(counters[current_nid]))++;
          }

          // work stealing
          std::set<int> unfinished_numa;
          for (int n = 0; n < p_numa_nodes; n++) {
            if (n != nid) {
              unfinished_numa.insert(n);
            }
          }

          while (unfinished_numa.size() != 0) {
            for (auto it = unfinished_numa.begin();
                 it != unfinished_numa.end();) {
              int n = *it;
              uint64_t current_nid = (nid + n) % p_numa_nodes;

              auto* so_container = so_containers[current_nid];
              uint64_t old_count = (*(counters[current_nid]))++;
              if (old_count <= max_counters[current_nid]) {
                start = old_count * p_chunk;
                end = std::min(static_cast<uint64_t>(so_container->size()),
                               start + p_chunk);

                for (uint64_t i = start; i < end; ++i) {
                  function((*so_container)[i], SoHandle(current_nid, t, i));
                }
                ++it;
              } else {
                it = unfinished_numa.erase(it);
              }
            }
          }
        }

        for (int n = 0; n < numa_nodes_; n++) {
          delete counters[n];
        }
      });
    }
  }

  /// Reserves enough memory to hold `capacity` number of simulation objects for
  /// each simulation object type.
  void Reserve(size_t capacity) {
    ApplyOnAllTypes([&](auto* container, uint16_t numa_idx, uint16_t type_idx) {
      container->reserve(capacity);
    });
  }

  /// Resize `sim_objects_[numa_node]` such that it holds `current + additional`
  /// elements after this call.
  /// Returns the size after
  uint64_t GrowSoContainer(size_t additional, size_t numa_node,
                           size_t type_id) {
    uint64_t current = 0;
    ::bdm::Apply(&sim_objects_[numa_node], type_id, [&](auto* container) {
      current = container->size();
      if (additional == 0) {
        return;
      }
      container->resize(current + additional);
    });
    return current;
  }

  /// Reserves enough memory to hold `capacity` number of simulation objects for
  /// the given simulation object type.
  template <typename TSo>
  void Reserve(size_t capacity) {
    for (uint16_t n = 0; n < numa_nodes_; n++) {
      GetContainer<TSo>(n)->reserve(capacity);
    }
  }

  /// Returns true if a sim object with the given uid is stored in this
  /// ResourceManager.
  bool Contains(SoUid uid) const {
    return so_storage_location_.find(uid) != so_storage_location_.end();
  }

  /// Remove all simulation objects
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Clear() {
    so_storage_location_.clear();
    ApplyOnAllTypes([](auto* container, uint16_t numa_node, uint16_t type_idx) {
      container->clear();
    });
  }

  /// Reorder simulation objects such that, sim objects are distributed to NUMA
  /// nodes. Nearby sim objects will be moved to the same NUMA node.
  void SortAndBalanceNumaNodes() {
    // balance simulation objects per numa node according to the number of
    // threads associated with each numa domain
    std::vector<uint64_t> so_per_numa(numa_nodes_);
    uint64_t cummulative = 0;
    auto max_threads = thread_info_->GetMaxThreads();
    for (int n = 1; n < numa_nodes_; ++n) {
      auto threads_in_numa = thread_info_->GetThreadsInNumaNode(n);
      uint64_t num_so = GetNumSimObjects() * threads_in_numa / max_threads;
      so_per_numa[n] = num_so;
      cummulative += num_so;
    }
    so_per_numa[0] = GetNumSimObjects() - cummulative;

    // using first touch policy - page will be allocated to the numa domain of
    // the thread that accesses it first.
    // alternative, use numa_alloc_onnode.
    int ret = numa_run_on_node(0);
    if (ret != 0) {
      Log::Fatal("ResourceManager", "Run on numa node failed. Return code: ",
                 ret);
    }

    TupleOfSOContainers* so_rearranged = new TupleOfSOContainers[numa_nodes_];
    TupleOfSOContainers* tmp = sim_objects_;
    sim_objects_ = so_rearranged;
    Clear();
    sim_objects_ = tmp;

    // numa node -> type idx -> vector of SoHandles
    std::vector<std::vector<std::vector<SoHandle>>> sorted_so_handles;
    sorted_so_handles.resize(numa_nodes_);
    for (auto& e : sorted_so_handles) {
      e.resize(NumberOfTypes());
    }

    uint64_t cnt = 0;
    uint64_t current_numa = 0;

    auto rearrange = [&](const SoHandle& handle) {
      if (cnt == so_per_numa[current_numa]) {
        cnt = 0;
        current_numa++;
      }

      sorted_so_handles[current_numa][handle.GetTypeIdx()].push_back(handle);
      cnt++;
    };

    auto* grid = Simulation<TCompileTimeParam>::GetActive()->GetGrid();
    grid->IterateZOrder(rearrange);

    for (int n = 0; n < numa_nodes_; n++) {
      for (uint16_t t = 0; t < NumberOfTypes(); t++) {
        ::bdm::Apply(&so_rearranged[n], t, [&, this](auto* dest) {
          std::atomic<bool> resized(false);
#pragma omp parallel
          {
            auto tid = omp_get_thread_num();
            auto nid = thread_info_->GetNumaNode(tid);
            if (nid == n) {
              auto old = std::atomic_exchange(&resized, true);
              if (!old) {
                dest->resize(sorted_so_handles[n][t].size());
              }
            }
          }
#pragma omp parallel
          {
            auto tid = omp_get_thread_num();
            auto nid = thread_info_->GetNumaNode(tid);
            if (nid == n) {
              auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
              auto& sohandles = sorted_so_handles[n][t];
              assert(thread_info_->GetNumaNode(tid) ==
                     numa_node_of_cpu(sched_getcpu()));

              // use static scheduling
              auto correction = sohandles.size() % threads_in_numa == 0 ? 0 : 1;
              auto chunk = sohandles.size() / threads_in_numa + correction;
              auto start = thread_info_->GetNumaThreadId(tid) * chunk;
              auto end = std::min(sohandles.size(), start + chunk);

              for (uint64_t e = start; e < end; e++) {
                auto& handle = sohandles[e];
                this->ApplyOnElement(handle, [&](auto&& sim_object) {
                  using So = raw_type<decltype(sim_object)>;
                  using SoBackend = typename So::Backend;
                  using DestValueType =
                      typename raw_type<decltype(dest)>::value_type;
                  using DestScalarSoType =
                      typename DestValueType::template Self<Scalar>;
                  using ScalarSo = typename So::template Self<Scalar>;

                  if (std::is_same<DestScalarSoType, ScalarSo>::value) {
                    auto&& tmp = reinterpret_cast<
                        typename DestValueType::template Self<SoBackend>&&>(
                        sim_object);
                    (*dest)[e] = tmp;
                    auto&& so = (*dest)[e];
                    so.SetNumaNode(n);
                    so.SetElementIdx(e);
                  }
                });
              }
            }
          }
        });
      }
    }

    // TODO(lukas) for AOS deleting takes around 1s for 3M objects - think about
    // delaying deletion - does not happen right away.
    // auto* to_be_deleted = sim_objects_;
    // std::async(std::launch::async, [=]{ delete[] to_be_deleted; });
    delete[] sim_objects_;
    sim_objects_ = so_rearranged;

    // update so_storage_location_
    ApplyOnAllElements([this](auto&& so, SoHandle) {
      this->so_storage_location_[so.GetUid()] = so.GetSoHandle();
    });

    // FIXME
    // // TODO(lukas) do we need this? we don't change the scheduling anymore
    thread_info_->Renew();
  }

  /// NB: This method is not thread-safe! This function might invalidate
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  template <typename TSo>
  void push_back(const TSo& so) {  // NOLINT
    typename SoHandle::NumaNode_t numa_node = 0;
    auto* container = GetContainer<TSo>(numa_node);
    container->push_back(so);
    auto el_idx = container->size() - 1;
    auto&& inserted = (*container)[el_idx];
    inserted.SetNumaNode(numa_node);
    inserted.SetElementIdx(el_idx);
    so_storage_location_[inserted.GetUid()] =
        SoHandle(numa_node, GetTypeIndex<TSo>(), el_idx);
  }

  /// Adds `new_sim_objects` to `sim_objects_[numa_node]`. `offset` specifies
  /// the index at which the first element is inserted. Sim objects are inserted
  /// consecutively. This methos is thread safe only if insertion intervals do
  /// not overlap! \n
  /// This method does not update `so_storage_location_`.
  /// \see `AddNewSimObjectsToSoStorageMap`
  void AddNewSimObjects(typename SoHandle::NumaNode_t numa_node,
                        typename SoHandle::TypeIdx_t type_id, uint64_t offset,
                        ResourceManager& other_rm) {
    // ::bdm::Apply(&other_rm.sim_objects_[numa_node], type_id, [&,this](auto*
    // new_sim_objects) {
    ::bdm::Apply(&sim_objects_[numa_node], type_id, [&, this](auto* container) {
      using SoType = typename raw_type<decltype(container)>::value_type;
      auto* new_sim_objects = other_rm.template Get<SoType>(numa_node);
      for (uint64_t i = 0; i < new_sim_objects->size(); i++) {
        auto&& so = (*new_sim_objects)[i];
        auto uid = so.GetUid();
        // so_storage_location_[uid] = SoHandle(numa_node, type_id, offset + i);
        (*container)[offset + i] = so;
      }
    });
  }

  /// Second part of adding simulation objects to this ResourceManager.
  /// NB: This method is not thread-safe!
  /// Part 1 \see AddNewSimObjects
  void AddNewSimObjectsToSoStorageMap(typename SoHandle::NumaNode_t numa_node,
                                      typename SoHandle::TypeIdx_t type_id,
                                      uint64_t offset,
                                      ResourceManager& other_rm) {
    // ::bdm::Apply(&other_rm.sim_objects_[numa_node], type_id, [&,this](auto*
    // new_sim_objects) {
    ::bdm::Apply(&sim_objects_[numa_node], type_id, [&, this](auto* container) {
      using SoType = typename raw_type<decltype(container)>::value_type;
      auto* new_sim_objects = other_rm.template Get<SoType>(numa_node);
      for (uint64_t i = 0; i < new_sim_objects->size(); i++) {
        auto&& so = (*new_sim_objects)[i];
        auto uid = so.GetUid();
        so_storage_location_[uid] = SoHandle(numa_node, type_id, offset + i);
      }
    });
  }

  /// Removes the simulation object with the given uid.\n
  /// NB: This method is not thread-safe! This function invalidates
  /// sim_object references pointing into the ResourceManager. SoPointer are
  /// not affected.
  void Remove(SoUid uid) {
    // remove from map
    auto it = this->so_storage_location_.find(uid);
    if (it != this->so_storage_location_.end()) {
      SoHandle soh = it->second;
      auto type_idx = soh.GetTypeIdx();

      ::bdm::Apply(&sim_objects_[soh.GetNumaNode()], type_idx,
                   [&, this](auto* container) {
                     auto element_idx = soh.GetElementIdx();
                     auto last = container->size() - 1;
                     if (element_idx == last) {
                       container->pop_back();
                     } else {
                       (*container)[element_idx] = (*container)[last];
                       (*container)[element_idx].SetElementIdx(element_idx);
                       container->pop_back();
                       SoUid changed_uid = (*container)[element_idx].GetUid();
                       this->so_storage_location_[changed_uid] =
                           SoHandle(type_idx, element_idx);
                     }
                   });
      so_storage_location_.erase(it);
    }
  }

#ifdef USE_OPENCL
  cl::Context* GetOpenCLContext() { return &opencl_context_; }
  cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }
  std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }
  std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }
#endif

  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  const TypeContainer<ToBackend<Type>>* Get(
      typename SoHandle::NumaNode_t numa_node = 0) {
    return &std::get<TypeContainer<ToBackend<Type>>>(sim_objects_[numa_node]);
  }

 private:
  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  TypeContainer<ToBackend<Type>>* GetContainer(
      typename SoHandle::NumaNode_t numa = 0) {
    return &std::get<TypeContainer<ToBackend<Type>>>(sim_objects_[numa]);
  }

  /// Mapping between SoUid and SoHandle (stored location)
  std::unordered_map<SoUid, SoHandle> so_storage_location_;  //!

  ThreadInfo* thread_info_ = ThreadInfo::GetInstance();  //!

  /// Conversion of simulation object types from the compile time params
  /// (`SimObjectTypes`) to a tuple of containers:
  /// `std::tuple<Container<SimObject1>, Container<SimOBject2>>`\n
  /// Container type is determined based on the specified simulation backend
  using TupleOfSOContainers =
      typename ToTupleOfSOContainers<Backend, Types>::type;

  /// Simulation object storgage. \n
  /// For each NUMA node on this machine create a `TupleOfSOContainers`
  Int_t numa_nodes_ = 0;  // needed to help ROOT with array size
  TupleOfSOContainers* sim_objects_ = nullptr;  //[numa_nodes_]

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
