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

#ifndef UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_
#define UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_

#include "resource_manager.h"

#include <vector>
#include "backend.h"
#include "compile_time_list.h"
#include "compile_time_param.h"
#include "io_util.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "type_util.h"
#include "unit/test_sim_object.h"
#include "unit/test_util.h"

namespace bdm {

BDM_SIM_OBJECT(A, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(A, TestSimObject, 1, data_);

 public:
  AExt() {}  // for ROOT I/O
  explicit AExt(int data) { data_[kIdx] = data; }

  int GetData() { return data_[kIdx]; }
  void SetData(int data) { data_[kIdx] = data; }

  vec<int> data_;
};

BDM_SIM_OBJECT(B, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(B, TestSimObject, 1, data_);

 public:
  BExt() {}  // for ROOT I/O
  explicit BExt(double data) { data_[kIdx] = data; }

  double GetData() { return data_[kIdx]; }
  void SetData(double data) { data_[kIdx] = data; }

  vec<double> data_;
};

/// Create ResourceManager with two types, use Get function to obtain container
/// of the specified type, push_back values and check if they have correctly
/// been added inside the ResourceManager
template <typename TRm, typename TA, typename TB>
inline void RunGetTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  // template specifier needed because A is dependant type
  auto a_vector = rm.template Get<TA>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(TAScalar(12));
  a_vector->push_back(TAScalar(34));
  EXPECT_EQ(12, (*rm.template Get<TA>())[0].GetData());
  EXPECT_EQ(34, (*rm.template Get<TA>())[1].GetData());
  EXPECT_EQ(2u, rm.template Get<TA>()->size());

  auto b_vector = rm.template Get<TB>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(TBScalar(3.14));
  b_vector->push_back(TBScalar(6.28));
  EXPECT_NEAR(3.14, (*rm.template Get<TB>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*rm.template Get<TB>())[1].GetData(), kEpsilon);
  EXPECT_EQ(2u, rm.template Get<TB>()->size());
}

template <typename TRm, typename TA, typename TB>
inline void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;
  rm.Clear();

  auto a_collection = rm.template Get<TA>();
  a_collection->push_back(TAScalar(12));
  a_collection->push_back(TAScalar(34));
  rm.ApplyOnElement(SoHandle(0, 0, 1),
                    [](auto&& element) { EXPECT_EQ(34, element.GetData()); });

  auto b_collection = rm.template Get<TB>();
  b_collection->push_back(TBScalar(3.14));
  b_collection->push_back(TBScalar(6.28));
  rm.ApplyOnElement(SoHandle(0, 1, 0), [&](auto&& element) {
    EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
  });
}

template <typename TRm, typename TA, typename TB>
void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  auto a_collection = rm.template Get<TA>();
  a_collection->push_back(TAScalar(12));
  a_collection->push_back(TAScalar(34));

  auto b_collection = rm.template Get<TB>();
  b_collection->push_back(TBScalar(3.14));
  b_collection->push_back(TBScalar(6.28));
  size_t counter = 0;
  rm.ApplyOnAllElements([&](auto&& element, SoHandle&& handle) {  // NOLINT
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(12, element.GetData());
        break;
      case 2:
        EXPECT_EQ(34, element.GetData());
        break;
      case 3:
        EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
        break;
      case 4:
        EXPECT_NEAR(6.28, element.GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(4u, counter);
}

template <typename TRm, typename TA, typename TB>
void RunGetNumSimObjects() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  auto a_collection = rm.template Get<TA>();
  a_collection->push_back(TAScalar(12));
  a_collection->push_back(TAScalar(34));
  a_collection->push_back(TAScalar(59));

  auto b_collection = rm.template Get<TB>();
  b_collection->push_back(TBScalar(3.14));
  b_collection->push_back(TBScalar(6.28));

  EXPECT_EQ(5u, rm.GetNumSimObjects());
}

// This test uses Cells since SoaA, SoaB are strippted down simulatio objects
// and are themselves not thread safe.
template <typename TRm, typename TB>
void RunApplyOnAllElementsParallelTest() {
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  auto cells = rm.template Get<TB>();
  cells->push_back(TBScalar(3.14));
  cells->push_back(TBScalar(6.28));
  cells->push_back(TBScalar(9.42));

  rm.ApplyOnAllElementsParallel([](auto&& element, SoHandle handle) {
    const double kEpsilon = abs_error<double>::value;
    if (handle == SoHandle(0, 1, 0)) {
      EXPECT_EQ(3.14, element.GetData());
    } else if (handle == SoHandle(0, 1, 1)) {
      EXPECT_EQ(6.28, element.GetData());
    } else if (handle == SoHandle(0, 1, 2)) {
      EXPECT_NEAR(9.42, element.GetData(), kEpsilon);
    } else {
      FAIL();
    }
  });
}

template <typename TRm, typename TA, typename TB>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  auto a_collection = rm.template Get<TA>();
  a_collection->push_back(TAScalar(12));

  auto b_collection = rm.template Get<TB>();
  b_collection->push_back(TBScalar(3.14));
  b_collection->push_back(TBScalar(6.28));
  size_t counter = 0;
  rm.ApplyOnAllTypes([&](auto* container, uint16_t numa_node, uint16_t type_idx) {
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(1u, container->size());
        EXPECT_EQ(12, (*container)[0].GetData());
        break;
      case 2:
        EXPECT_EQ(2u, container->size());
        EXPECT_NEAR(3.14, (*container)[0].GetData(), kEpsilon);
        EXPECT_NEAR(6.28, (*container)[1].GetData(), kEpsilon);
        break;
    }
  });

  ThreadInfo ti;
  EXPECT_EQ(ti.GetNumaNodes() * 2u, counter);
}

template <typename TRm, typename TA, typename TB>
void RunGetTypeIndexTest() {
  TRm rm;
  EXPECT_EQ(0u, TRm::template GetTypeIndex<TA>());
  EXPECT_EQ(1u, TRm::template GetTypeIndex<TB>());
}

template <typename TRm, typename TA, typename TB>
void RunPushBackTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));

  rm.push_back(TAScalar(87));

  auto as = rm.template Get<TA>();
  auto bs = rm.template Get<TB>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);
}

template <typename TRm, typename TA, typename TB>
void RunNewTest() {
  const double kEpsilon = abs_error<double>::value;
  TRm rm;

  auto* as = rm.template Get<TA>();
  auto* bs = rm.template Get<TB>();
  // TODO(lukas) Remove reserves after https://trello.com/c/sKoOTgJM has been
  // resolved
  as->reserve(5);
  bs->reserve(5);

  auto&& a0 = rm.template New<A>(12);
  auto&& a1 = rm.template New<A>(34);

  auto&& b0 = rm.template New<B>(3.14);
  auto&& b1 = rm.template New<B>(6.28);

  auto&& a2 = rm.template New<A>(87);

  EXPECT_EQ(a0.GetData(), 12);
  EXPECT_EQ(a1.GetData(), 34);
  EXPECT_EQ(a2.GetData(), 87);

  EXPECT_NEAR(b0.GetData(), 3.14, kEpsilon);
  EXPECT_NEAR(b1.GetData(), 6.28, kEpsilon);

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);

  // modify return value of new
  a1.SetData(321);
  EXPECT_EQ((*as)[1].GetData(), 321);
}


// -----------------------------------------------------------------------------
// https://github.com/osmhpi/pgasus/blob/775a5f90d8f6fa89cfb93eac6de16dcfe27167ce/src/util/mmaphelper.cpp
inline static void* AlignPage(const void *ptr) {
  static constexpr uintptr_t PAGE_MASK = ~(uintptr_t(0xFFF));
  return (void*) (((uintptr_t)ptr) & PAGE_MASK);
}

inline int GetNumaNodeForMemory(const void *ptr) {
  int result, loc;
  void *pptr = AlignPage(ptr);
  result = numa_move_pages(0, 1, &pptr, nullptr, &loc, 0);
  return (result != 0) ? -1 : loc;
}

inline std::vector<uint64_t> GetSoPerNuma(uint64_t num_sim_objects) {
  // balance simulation objects per numa node according to the number of
  // threads associated with each numa domain
  ThreadInfo ti;
  int numa_nodes = ti.GetNumaNodes();

  std::vector<uint64_t> so_per_numa(numa_nodes);
  uint64_t cummulative = 0;
  auto max_threads = ti.GetMaxThreads();
  for (int n = 1; n < numa_nodes; ++n) {
    auto threads_in_numa = ti.GetThreadsInNumaNode(n);
    uint64_t num_so = num_sim_objects * threads_in_numa / max_threads;
    so_per_numa[n] = num_so;
    cummulative += num_so;
  }
  so_per_numa[0] = num_sim_objects - cummulative;
  return so_per_numa;
}

// -----------------------------------------------------------------------------
template <typename TRm>
inline void CheckApplyOnAllElements(TRm* rm, uint64_t num_sim_objects, bool numa_checks = false) {
  std::vector<bool> found(2 * num_sim_objects);
  ASSERT_EQ(2 * num_sim_objects, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    found[i] = false;
  }

  std::atomic<uint64_t> cnt(0);
  ThreadInfo ti;
  // counts the number of sim objects in each numa domain
  std::vector<uint64_t> numa_so_cnts;
  numa_so_cnts.resize(ti.GetNumaNodes());
  std::atomic<uint64_t> numa_memory_errors(0);
  std::atomic<uint64_t> numa_thread_errors(0);

  rm->ApplyOnAllElementsParallel([&](auto&& so, const SoHandle& handle) {
    size_t index = std::round(so.GetData());
    if(handle != so.GetSoHandle()) {
      FAIL() << "handle != so.GetSoHandle()";
    }

    #pragma omp critical
    {
      found[index] = true;

      // verify that a thread processes sim objects on the same NUMA node.
      if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(&so)) {
        numa_memory_errors++;
      }
      if (numa_checks && handle.GetNumaNode() != numa_node_of_cpu(sched_getcpu())) {
        numa_thread_errors++;
      }

      numa_so_cnts[handle.GetNumaNode()]++;
    }
    cnt++;
  });

  EXPECT_EQ(2 * num_sim_objects, cnt.load());
  ASSERT_EQ(2 * num_sim_objects, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    if (!found[i]) {
      FAIL() << "ApplyOnAllElementsParallel was not called for element with data_=" << i;
    }
  }

  if (numa_checks) {
    if(!std::is_same<Soa, typename raw_type<decltype(rm)>::Backend>::value) {
      EXPECT_EQ(0u, numa_memory_errors.load());
    }
    EXPECT_EQ(0u, numa_thread_errors.load());
    auto so_per_numa = GetSoPerNuma(2 * num_sim_objects);
    for(int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallel(uint64_t num_sim_objects) {
  TSimulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  for(uint64_t i = 0; i < num_sim_objects; ++i) {
    TA a(i);
    a.SetPosition({i * 30.0, 0, 0});
    rm->push_back(a);
    TB b(i+num_sim_objects);
    b.SetPosition({i * 30.0, 0, 0});
    rm->push_back(b);
  }

  CheckApplyOnAllElements(rm, num_sim_objects);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElements(rm, num_sim_objects, true);
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallel() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_sim_objects = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};

  for(auto n : num_sim_objects) {
    RunSortAndApplyOnAllElementsParallel<A, B>(n);
  }

  RunSortAndApplyOnAllElementsParallel<A, B>(1000);
}

// -----------------------------------------------------------------------------
template <typename TRm>
inline void CheckApplyOnAllElementsDynamic(TRm* rm, uint64_t num_sim_objects, uint64_t batch_size, bool numa_checks = false) {
  std::vector<bool> found(2 * num_sim_objects);
  ASSERT_EQ(2 * num_sim_objects, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    found[i] = false;
  }

  std::atomic<uint64_t> cnt(0);
  ThreadInfo ti;
  // counts the number of sim objects in each numa domain
  std::vector<uint64_t> numa_so_cnts;
  numa_so_cnts.resize(ti.GetNumaNodes());
  std::atomic<uint64_t> numa_memory_errors(0);
  std::atomic<uint64_t> numa_thread_errors(0);

  rm->ApplyOnAllElementsParallelDynamic(batch_size, [&](auto&& so, const SoHandle& handle) {
    size_t index = std::round(so.GetData());
    if(handle != so.GetSoHandle()) {
      FAIL() << "handle != so.GetSoHandle()";
    }

    #pragma omp critical
    {
      found[index] = true;

      // verify that a thread processes sim objects on the same NUMA node.
      if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(&so)) {
        numa_memory_errors++;
      }
      if (numa_checks && handle.GetNumaNode() != numa_node_of_cpu(sched_getcpu())) {
        numa_thread_errors++;
      }

      numa_so_cnts[handle.GetNumaNode()]++;
    }
    cnt++;
  });

  // verify that the function has been called once for each sim object
  EXPECT_EQ(2 * num_sim_objects, cnt.load());
  ASSERT_EQ(2 * num_sim_objects, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    if (!found[i]) {
      FAIL() << "ApplyOnAllElementsParallel was not called for element with data_=" << i;
    }
  }

  if (numa_checks) {
    if(!std::is_same<Soa, typename raw_type<decltype(rm)>::Backend>::value) {
      EXPECT_EQ(0u, numa_memory_errors.load());
    }
    EXPECT_EQ(0u, numa_thread_errors.load());
    auto so_per_numa = GetSoPerNuma(2 * num_sim_objects);
    for(int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallelDynamic(uint64_t num_sim_objects, uint64_t batch_size) {
  TSimulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  for(uint64_t i = 0; i < num_sim_objects; ++i) {
      TA a(i);
      a.SetPosition({i * 30.0, 0, 0});
      rm->push_back(a);
      TB b(i+num_sim_objects);
      b.SetPosition({i * 30.0, 0, 0});
      rm->push_back(b);
  }

  CheckApplyOnAllElementsDynamic(rm, num_sim_objects, batch_size);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElementsDynamic(rm, num_sim_objects, batch_size, true);
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallelDynamic() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_sim_objects = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};
  std::vector<int> batch_sizes = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};

  for(auto n : num_sim_objects) {
    for(auto b : batch_sizes) {
      RunSortAndApplyOnAllElementsParallelDynamic<A, B>(n, b);
    }
  }

  for(auto b : batch_sizes) {
    RunSortAndApplyOnAllElementsParallelDynamic<A, B>(num_threads * 1000, b);
  }
  // RunSortAndApplyOnAllElementsParallelDynamic<A, B>(144000, 100, true);
  // RunSortAndApplyOnAllElementsParallelDynamic<A, B>(100000, 1, true);
}

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_
