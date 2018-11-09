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
#include <sched.h>
#include <numa.h>
#include <omp.h>
#include <iomanip>  // TODO remove (used for temporary function)
#include <strstream>  // TODO remove (used for temporary function)
#include <fstream>  // TODO remove (used for temporary function)

#ifdef USE_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif

#include "root_util.h"
#include "backend.h"
#include "compile_time_list.h"
#include "diffusion_grid.h"
#include "simulation.h"
#include "thread_info.h"
#include "tuple_util.h"

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
        Log::Fatal("ResourceManager", "Call to numa_available failed with return code: ", ret);
    }
    // create a sim object storage instance for each numa node
    // sim_objects_.resize(numa_num_configured_nodes());  // FIXME
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
    // FIXME
    // sim_objects_ = std::move(other.sim_objects_);
    diffusion_grids_ = std::move(other.diffusion_grids_);
    return *this;
  }

  // FIXME make private?
  /// Return the container of this Type
  /// @tparam Type atomic type whose container should be returned
  ///         invariant to the Backend. This means that even if ResourceManager
  ///         stores e.g. `SoaCell`, Type can be `Cell` and still returns the
  ///         correct container.
  template <typename Type>
  TypeContainer<ToBackend<Type>>* Get(typename SoHandle::NumaNode_t numa_node = 0) {
    return &std::get<TypeContainer<ToBackend<Type>>>(sim_objects_[numa_node]);
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

  uint16_t GetNumNumaNodes() {
    return numa_nodes_;
  }

  /// Returns the total number of simulation objects
  size_t GetNumSimObjects() {
    size_t num_so = 0;
    for(uint16_t n = 0; n < numa_nodes_; n++) {
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value; i++) {
        ::bdm::Apply(&sim_objects_[n], i,
                     [&](auto* container) { num_so += container->size(); });
      }
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
    auto numa_node = handle.GetNumaNode();
    auto element_idx = handle.GetElementIdx();
    return ::bdm::Apply(&sim_objects_[numa_node], type_idx, [&](auto* container) -> decltype(
                                              function((*container)[0])) {
      return function((*container)[element_idx]);
    });
  }
  // FIXME documentation in this class

  /// Apply a function on all container types
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, typename SoHandle::NumaNode_t numa_node, typename SoHandle::TypeIdx_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypes(TFunction&& function) {
    for(uint16_t n = 0; n < numa_nodes_; n++) {
      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value; i++) {
        ::bdm::Apply(&sim_objects_[n], i, [&](auto* container) { function(container, n, i); });
      }
    }
  }

  /// Apply a function on all container types. Function invocations are
  /// parallelized
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ApplyOnAllTypes([](auto* container, typename SoHandle::NumaNode_t numa_node, typename SoHandle::TypeIdx_t type_idx) {
  ///                          std::cout << container->size() << std::endl;
  ///                        });
  template <typename TFunction>
  void ApplyOnAllTypesParallel(TFunction&& function) {
    // FIXME this is not parallel
    for(uint16_t n = 0; n < numa_nodes_; n++) {
      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value; i++) {
        ::bdm::Apply(&sim_objects_[n], i, [&](auto* container) { function(container, n, i); });
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
    for(uint16_t n = 0; n < numa_nodes_; n++) {
      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value; i++) {
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

    omp_set_nested(1); // TODO move to generatl openmp initialization

    #pragma omp parallel num_threads(nodes)
    {
       auto numa_thread_id = omp_get_thread_num();
       numa_run_on_node(numa_thread_id);

      // runtime dispatch - TODO(lukas) replace with c++17 std::apply
      for (uint16_t i = 0; i < std::tuple_size<TupleOfSOContainers>::value; i++) {
        ::bdm::Apply(&sim_objects_[numa_thread_id], i, [&](auto* container) {
  #pragma omp parallel num_threads(cores_per_node)
        {
          //  #pragma omp critical
          //  std::cout << "numa " << numa_thread_id << " type " << i << " num elements " << container->size() << std::endl;
           #pragma omp for //schedule(dynamic, 100)
          for (size_t e = 0; e < container->size(); e++) {
            function((*container)[e], SoHandle(numa_thread_id, i, e));
          }
        }
        });
      }
    }
  }

  template <typename TFunction>
  void ApplyOnAllElementsParallel(TFunction&& function) {
    for (uint16_t t = 0; t < NumberOfTypes(); ++t) {
      // only needed to get the type of the container
      ::bdm::Apply(&sim_objects_[0], t, [&](auto* container) {
        // collect all containers of this type
        std::vector<decltype(container)> so_containers;
        so_containers.resize(numa_nodes_);
        so_containers[0] = container;
        for(uint16_t n = 1; n < numa_nodes_; n++) {
          ::bdm::Apply(&sim_objects_[n], t, [&](auto* container) {
            so_containers[n] = container;
          });
        }

        #pragma omp parallel
        {
          auto tid = omp_get_thread_num();
          auto nid = thread_info_.GetNumaNode(tid);
          auto threads_in_numa = thread_info_.GetThreadsInNumaNode(nid);
          auto* so_container = so_containers[nid];
          assert(thread_info_.GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

          // use static scheduling for now
          auto correction = so_container->size() % threads_in_numa == 0 ? 0 : 1;
          auto chunk = so_container->size() / threads_in_numa + correction;
          auto start = thread_info_.GetNumaThreadId(tid) * chunk ;
          auto end = std::min(so_container->size(), start + chunk);

          // #pragma omp critical
          // {
          //   std::cout << "tid             " << tid << std::endl;
          //   std::cout << "nid             " << nid << std::endl;
          //   std::cout << "ntid            " << thread_info_.GetNumaThreadId(tid) << std::endl;
          //   std::cout << "cont size       " << so_container->size() << std::endl;
          //   std::cout << "threads in numa " << threads_in_numa << std::endl;
          //   std::cout << "chunk           " << chunk << std::endl;
          //   std::cout << "start           " << start << std::endl;
          //   std::cout << "end             " << end << std::endl << std::endl;
          // }

          for(uint64_t i = start; i < end; ++i) {
            function((*so_container)[i], SoHandle(nid, t, i));
          }
        }
      });
    }
  }

  template <typename TFunction>
  void ApplyOnAllElementsParallelDynamic(uint64_t chunk, TFunction&& function) {
    for (uint16_t t = 0; t < NumberOfTypes(); ++t) {
      // only needed to get the type of the container
      ::bdm::Apply(&sim_objects_[0], t, [&](auto* container) {
        // collect all containers of this type
        std::vector<decltype(container)> so_containers;
        so_containers.resize(numa_nodes_);
        so_containers[0] = container;
        for(uint16_t n = 1; n < numa_nodes_; n++) {
          ::bdm::Apply(&sim_objects_[n], t, [&](auto* container) {
            so_containers[n] = container;
          });
        }

        // use dynamic scheduling
        // Unfortunately openmp's built in functionality can't be used, since
        // threads belong to different numa domains and thus operate on
        // different containers
        std::vector<std::atomic<uint64_t>*> counters(numa_nodes_, nullptr);
        std::vector<uint64_t> max_counters(numa_nodes_);
        for(uint64_t n = 0; n < numa_nodes_; n++) {
          counters[n] = new std::atomic<uint64_t>(0);
          // calculate value max_counters for each numa domain
          auto correction = so_containers[n]->size() % chunk == 0 ? 0 : 1;
          max_counters[n] = so_containers[n]->size() / chunk + correction;
        }

        #pragma omp parallel firstprivate(chunk, numa_nodes_)
        {
          auto tid = omp_get_thread_num();
          auto nid = thread_info_.GetNumaNode(tid);
          assert(thread_info_.GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));


          // dynamic scheduling
          uint64_t start = 0;
          uint64_t end = 0;

          // this loop implements work stealing from other NUMA nodes if there
          // are imbalances. Each thread starts with its NUMA domain. Once, it
          // is finished the thread looks for tasks on other domains
          for(uint64_t n = 0; n < numa_nodes_; n++) {
            uint64_t current_nid = (nid + n) % numa_nodes_;

            auto* so_container = so_containers[current_nid];
            uint64_t old_count = (*(counters[current_nid]))++;
            while(old_count <= max_counters[current_nid]) {
              start = old_count * chunk ;
              end = std::min(so_container->size(), start + chunk);

              // #pragma omp critical
              // {
              //   std::cout << "tid             " << tid << std::endl;
              //   std::cout << "nid             " << nid << std::endl;
              //   std::cout << "current nid     " << current_nid << std::endl;
              //   std::cout << "cont size       " << so_container->size() << std::endl;
              //   std::cout << "old_count       " << old_count << std::endl;
              //   std::cout << "mac count       " << max_counters[current_nid] << std::endl;
              //   std::cout << "start           " << start << std::endl;
              //   std::cout << "end             " << end << std::endl << std::endl;
              // }

              for(uint64_t i = start; i < end; ++i) {
                function((*so_container)[i], SoHandle(current_nid, t, i));
              }

              old_count = (*(counters[current_nid]))++;
          }
        }
      }

          for(uint64_t n = 0; n < numa_nodes_; n++) {
            delete counters[n];
          }
      });
    }
  }

  /// Remove elements from each type
  void Clear() {
    ApplyOnAllTypes(
        [](auto* container, uint16_t numa_node, uint16_t type_idx) { container->clear(); });
  }

  // https://github.com/osmhpi/pgasus/blob/775a5f90d8f6fa89cfb93eac6de16dcfe27167ce/src/util/mmaphelper.cpp
  inline static void* align_page(const void *ptr) {
  	static constexpr uintptr_t PAGE_MASK = ~(uintptr_t(0xFFF));
  	return (void*) (((uintptr_t)ptr) & PAGE_MASK);
  }

  int getNumaNodeForMemory(const void *ptr) {
  	int result, loc;
  	void *pptr = align_page(ptr);

  	result = numa_move_pages(0, 1, &pptr, nullptr, &loc, 0);

  	return (result != 0) ? -1 : loc;
  }

  void PrintThreadCPUBinding(std::string filename) {
    std::vector<std::string> mapping;
    #pragma omp parallel
    {
      auto tid = omp_get_thread_num();
      #pragma omp critical
      {
        std::stringstream str;
        str << "tid " << std::setfill('0') << std::setw(3) << tid << " cpu " << std::setfill('0') << std::setw(3) << sched_getcpu() << std::endl;
        mapping.push_back(str.str());
      }
    }
    std::sort(mapping.begin(), mapping.end());
    std::ofstream ofs(filename);
    for(auto& e : mapping) {
      ofs << e;
    }
  }

  void SortAndBalanceNumaNodes() {
    PrintThreadCPUBinding("before");

    uint64_t so_per_numa = GetNumSimObjects() / numa_nodes_;
    uint64_t cnt = 0;
    uint64_t current_numa = 0;

    // using first touch policy - page will be allocated to the numa domain of
    // the thread that accesses it first.
    // alternative, use numa_alloc_onnode.
    int ret = numa_run_on_node(0);
    if(ret != 0) {
      Log::Fatal("ResourceManager", "Run on numa node failed. Return code: ", ret);
    }
    // std::cout << " sched_getcpu " <<  sched_getcpu()<< " numa of cpu: " << numa_node_of_cpu(sched_getcpu()) << std::endl;

    TupleOfSOContainers* so_rearranged = new TupleOfSOContainers[numa_nodes_];
    TupleOfSOContainers* tmp = sim_objects_;
    sim_objects_ = so_rearranged;
    Clear();
    sim_objects_ = tmp;

    // TODO reserve memory upfront to be more efficient

    auto rearrange  = [&](const SoHandle& handle) {
      if(cnt == so_per_numa) {
        cnt = 0;
        current_numa++;
        // change this threads numa domain
        int ret = numa_run_on_node(current_numa);
        if(ret != 0) {
          Log::Fatal("ResourceManager", "Run on numa node failed. Return code: ", ret);
        }
        // if (numa_node_of_cpu(sched_getcpu()) != )
      }
      ApplyOnElement(handle, [&](auto&& sim_object) {
        ::bdm::Apply(&so_rearranged[current_numa], handle.GetTypeIdx(), [&](auto* container) {
            container->push_back(sim_object);
            uint32_t element_idx = container->size() - 1;
            auto&& so = (*container)[element_idx];
            so.SetNumaNode(current_numa);
            so.SetElementIdx(element_idx);
        });
      });
      cnt++;
    };

      auto* grid = Simulation<TCompileTimeParam>::GetActive()->GetGrid();
      grid->IterateZOrder(rearrange);

    delete[] sim_objects_;
    sim_objects_ = so_rearranged;

    // checks
    ApplyOnAllTypes([](auto* container, uint16_t numa, uint16_t type_idx){
      std::cout << container->size() << std::endl;
    });

    uint64_t errcnt = 0;
    ApplyOnAllElements([&errcnt, this](auto&& so, const SoHandle& handle){
      auto node = getNumaNodeForMemory(&so);
      if(node != handle.GetNumaNode()) {
        errcnt++;
      }
    });
    PrintThreadCPUBinding("after");
    std::cout << "ERROR number " << errcnt << std::endl;

    thread_info_.Renew();
  }

  template <typename TSo>
  void push_back(const TSo& so) {  // NOLINT
    Get<TSo>(0)->push_back(so);
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
    typename SoHandle::NumaNode_t numa_node = 0;
    auto container = Get<TScalarSo>(numa_node);
    auto idx =
        container->DelayedPushBack(TScalarSo(std::forward<Args>(args)...));
    (*container)[idx].SetNumaNode(numa_node);
    return (*container)[idx];
  }

  template <typename TScalarSo, typename... Args, typename TBackend = Backend>
  typename std::enable_if<std::is_same<TBackend, Scalar>::value,
                          TScalarSo&>::type
  New(Args... args) {
    typename SoHandle::NumaNode_t numa_node = 0;
    auto container = Get<TScalarSo>(numa_node);
    auto idx =
        container->DelayedPushBack(TScalarSo(std::forward<Args>(args)...));
    (*container)[idx].SetNumaNode(numa_node);
    return (*container)[idx];
  }

 private:
   ThreadInfo thread_info_;  //!

  /// Conversion of simulation object types from the compile time params
  /// (`SimObjectTypes`) to a tuple of containers:
  /// `std::tuple<Container<SimObject1>, Container<SimOBject2>>`\n
  /// Container type is determined based on the specified simulation backend
  using TupleOfSOContainers = typename ToTupleOfSOContainers<Backend, Types>::type;

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

#endif  // RESOURCE_MANAGER_H_
