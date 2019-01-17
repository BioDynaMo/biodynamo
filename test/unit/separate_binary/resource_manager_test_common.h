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

  int GetData() const { return data_[kIdx]; }
  void SetData(int data) { data_[kIdx] = data; }

  vec<int> data_;
};

BDM_SIM_OBJECT(B, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(B, TestSimObject, 1, data_);

 public:
  BExt() {}  // for ROOT I/O
  explicit BExt(double data) { data_[kIdx] = data; }

  double GetData() const { return data_[kIdx]; }
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
  EXPECT_EQ(0u, rm.template Get<TA>()->size());
  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  EXPECT_EQ(2u, rm.template Get<TA>()->size());
  EXPECT_EQ(12, (*rm.template Get<TA>())[0].GetData());
  EXPECT_EQ(34, (*rm.template Get<TA>())[1].GetData());

  EXPECT_EQ(0u, rm.template Get<TB>()->size());
  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  EXPECT_EQ(2u, rm.template Get<TB>()->size());
  EXPECT_NEAR(3.14, (*rm.template Get<TB>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*rm.template Get<TB>())[1].GetData(), kEpsilon);
}

template <typename TRm, typename TA, typename TB>
inline void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;
  rm.Clear();

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  rm.ApplyOnElement(SoHandle(0, 0, 1),
                    [](auto&& element) { EXPECT_EQ(34, element.GetData()); });

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
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

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
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

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  rm.push_back(TAScalar(59));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));

  EXPECT_EQ(5u, rm.GetNumSimObjects());
}

// This test uses Cells since SoaA, SoaB are strippted down simulatio objects
// and are themselves not thread safe.
template <typename TRm, typename TB>
void RunApplyOnAllElementsParallelTest() {
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  rm.push_back(TBScalar(9.42));

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
void RunRemoveAndContainsTest() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  TAScalar a0(12);
  auto a0_uid = a0.GetUid();
  rm.push_back(a0);

  TAScalar a1(34);
  auto a1_uid = a1.GetUid();
  rm.push_back(a1);

  TAScalar a2(59);
  auto a2_uid = a2.GetUid();
  rm.push_back(a2);

  TBScalar b0(3.14);
  auto b0_uid = b0.GetUid();
  rm.push_back(b0);

  TBScalar b1(6.28);
  auto b1_uid = b1.GetUid();
  rm.push_back(b1);

  EXPECT_TRUE(rm.Contains(a0_uid));
  EXPECT_TRUE(rm.Contains(a1_uid));
  EXPECT_TRUE(rm.Contains(a2_uid));
  EXPECT_TRUE(rm.Contains(b0_uid));
  EXPECT_TRUE(rm.Contains(b1_uid));

  rm.Remove(a0_uid);
  rm.Remove(a1_uid);
  rm.Remove(a2_uid);
  rm.Remove(b0_uid);
  rm.Remove(b1_uid);

  EXPECT_FALSE(rm.Contains(a0_uid));
  EXPECT_FALSE(rm.Contains(a1_uid));
  EXPECT_FALSE(rm.Contains(a2_uid));
  EXPECT_FALSE(rm.Contains(b0_uid));
  EXPECT_FALSE(rm.Contains(b1_uid));

  EXPECT_EQ(0u, rm.GetNumSimObjects());
}

template <typename TRm, typename TA, typename TB>
void RunClearTest() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  TAScalar a0(12);
  auto a0_uid = a0.GetUid();
  rm.push_back(a0);

  TAScalar a1(34);
  auto a1_uid = a1.GetUid();
  rm.push_back(a1);

  TAScalar a2(59);
  auto a2_uid = a2.GetUid();
  rm.push_back(a2);

  TBScalar b0(3.14);
  auto b0_uid = b0.GetUid();
  rm.push_back(b0);

  TBScalar b1(6.28);
  auto b1_uid = b1.GetUid();
  rm.push_back(b1);

  EXPECT_TRUE(rm.Contains(a0_uid));
  EXPECT_TRUE(rm.Contains(a1_uid));
  EXPECT_TRUE(rm.Contains(a2_uid));
  EXPECT_TRUE(rm.Contains(b0_uid));
  EXPECT_TRUE(rm.Contains(b1_uid));

  rm.Clear();

  EXPECT_FALSE(rm.Contains(a0_uid));
  EXPECT_FALSE(rm.Contains(a1_uid));
  EXPECT_FALSE(rm.Contains(a2_uid));
  EXPECT_FALSE(rm.Contains(b0_uid));
  EXPECT_FALSE(rm.Contains(b1_uid));

  EXPECT_EQ(0u, rm.GetNumSimObjects());
}

template <typename TRm, typename TA, typename TB>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
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

  const auto* as = rm.template Get<TA>();
  const auto* bs = rm.template Get<TB>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);
}

template <typename TA, typename TB, typename TRm = ResourceManager<>>
void RunGetSimObjectTest() {
  TRm rm;
  auto current_uid = SoUidGenerator::Get()->NewSoUid();
  rm.push_back(TA(0));
  rm.push_back(TA(1));
  rm.push_back(TB(2));
  rm.push_back(TB(3));

  const double kEpsilon = abs_error<double>::value;
  EXPECT_EQ(0, rm.template GetSimObject<TA>(SoHandle(0, 0)).GetData());
  EXPECT_EQ(1, rm.template GetSimObject<TA>(SoHandle(0, 1)).GetData());
  EXPECT_NEAR(2, rm.template GetSimObject<TB>(SoHandle(1, 0)).GetData(),
              kEpsilon);
  EXPECT_NEAR(3, rm.template GetSimObject<TB>(SoHandle(1, 1)).GetData(),
              kEpsilon);

  EXPECT_EQ(0, rm.template GetSimObject<TA>(current_uid + 1).GetData());
  EXPECT_EQ(1, rm.template GetSimObject<TA>(current_uid + 2).GetData());
  EXPECT_NEAR(2, rm.template GetSimObject<TB>(current_uid + 3).GetData(),
              kEpsilon);
  EXPECT_NEAR(3, rm.template GetSimObject<TB>(current_uid + 4).GetData(),
              kEpsilon);
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
inline void CheckApplyOnAllElements(TRm* rm, uint64_t num_so_per_type, bool numa_checks = false) {
  std::vector<bool> found(2 * num_so_per_type);
  ASSERT_EQ(2 * num_so_per_type, found.size());
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
    ASSERT_EQ(handle, so.GetSoHandle());

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

  EXPECT_EQ(2 * num_so_per_type, cnt.load());
  ASSERT_EQ(2 * num_so_per_type, found.size());
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
    auto so_per_numa = GetSoPerNuma(2 * num_so_per_type);
    for(int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallel(uint64_t num_so_per_type) {
  TSimulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<SoUid, double> ta_x_values;
  std::unordered_map<SoUid, double> tb_x_values;
  for(uint64_t i = 0; i < num_so_per_type; ++i) {
    double x_pos = i * 30.0;

    TA a(i);
    a.SetPosition({x_pos, 0, 0});
    rm->push_back(a);
    ta_x_values[a.GetUid()] = x_pos;

    TB b(i+num_so_per_type);
    b.SetPosition({x_pos, 0, 0});
    rm->push_back(b);
    tb_x_values[b.GetUid()] = x_pos;
  }

  CheckApplyOnAllElements(rm, num_so_per_type);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElements(rm, num_so_per_type, true);

  // check if sim object uids still point to the correct object
  for(auto& entry : ta_x_values) {
    auto x_actual = rm->template GetSimObject<TA>(entry.first).GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for(auto& entry : tb_x_values) {
    auto x_actual = rm->template GetSimObject<TB>(entry.first).GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallel() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_so_per_type = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};

  for(auto n : num_so_per_type) {
    RunSortAndApplyOnAllElementsParallel<A, B>(n);
  }

  RunSortAndApplyOnAllElementsParallel<A, B>(1000);
}

// -----------------------------------------------------------------------------
template <typename TRm>
inline void CheckApplyOnAllElementsDynamic(TRm* rm, uint64_t num_so_per_type, uint64_t batch_size, bool numa_checks = false) {
  std::vector<bool> found(2 * num_so_per_type);
  ASSERT_EQ(2 * num_so_per_type, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    found[i] = false;
  }

  std::atomic<uint64_t> cnt(0);
  ThreadInfo ti;
  // counts the number of sim objects in each numa domain
  std::vector<uint64_t> numa_so_cnts;
  numa_so_cnts.resize(ti.GetNumaNodes());
  // If a simulation object is not stored on the NUMA indicated, it is a memory
  // error.
  std::atomic<uint64_t> numa_memory_errors(0);
  // If a sim object is processed by a thread that doesn't belong to the NUMA
  // domain the sim object is stored on, it is a thread error.
  std::atomic<uint64_t> numa_thread_errors(0);

  rm->ApplyOnAllElementsParallelDynamic(batch_size, [&](auto&& so, const SoHandle& handle) {
    #pragma omp critical
    {
      size_t index = std::round(so.GetData());
      found[index] = true;

      // verify that a thread processes sim objects on the same NUMA node.
      if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(&so)) {
        numa_memory_errors++;
      }

      numa_so_cnts[handle.GetNumaNode()]++;
    }
    ASSERT_EQ(handle, so.GetSoHandle());
    cnt++;
  });

  // critical sections increase the variance of numa_thread_errors.
  // Therefore, there are checked separately.
  rm->ApplyOnAllElementsParallelDynamic(batch_size, [&](auto&& so, const SoHandle& handle) {
    volatile double d = 0;
    for(int i = 0; i < 10000; i++) {
      d+= std::sin(i);
    }
    if (handle.GetNumaNode() != ti.GetNumaNode(omp_get_thread_num())) {
      numa_thread_errors++;
    }
  });

  // verify that the function has been called once for each sim object
  EXPECT_EQ(2 * num_so_per_type, cnt.load());
  ASSERT_EQ(2 * num_so_per_type, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    if (!found[i]) {
      FAIL() << "ApplyOnAllElementsParallel was not called for element with data_=" << i;
    }
  }

  if (numa_checks) {
    if(!std::is_same<Soa, typename raw_type<decltype(rm)>::Backend>::value) {
      // If there are memory errors, check of
      // `cat /proc/sys/kernel/numa_balancing` is zero.
      // Work stealing in combination with automatic rebalancing can lead to
      // numa memory errors.
      EXPECT_EQ(0u, numa_memory_errors.load());
    }
    // work stealing can cause thread errors. This check ensures that at least
    // 75% of the work is done by the correct CPU-Memory mapping.
    if (num_so_per_type > 20 * static_cast<uint64_t>(omp_get_max_threads())) {
      EXPECT_GT(num_so_per_type / 4, numa_thread_errors.load());
    }
    auto so_per_numa = GetSoPerNuma(2 * num_so_per_type);
    for(int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallelDynamic(uint64_t num_so_per_type, uint64_t batch_size) {
  TSimulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<SoUid, double> ta_x_values;
  std::unordered_map<SoUid, double> tb_x_values;
  for(uint64_t i = 0; i < num_so_per_type; ++i) {
      double x_pos = i * 30.0;
      
      TA a(i);
      a.SetPosition({x_pos, 0, 0});
      rm->push_back(a);
      ta_x_values[a.GetUid()] = x_pos;

      TB b(i+num_so_per_type);
      b.SetPosition({x_pos, 0, 0});
      rm->push_back(b);
      tb_x_values[b.GetUid()] = x_pos;
  }

  CheckApplyOnAllElementsDynamic(rm, num_so_per_type, batch_size);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElementsDynamic(rm, num_so_per_type, batch_size, true);

  // check if sim object uids still point to the correct object
  for(auto& entry : ta_x_values) {
    auto x_actual = rm->template GetSimObject<TA>(entry.first).GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for(auto& entry : tb_x_values) {
    auto x_actual = rm->template GetSimObject<TB>(entry.first).GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

template <typename TA, typename TB, typename TSimulation = Simulation<>>
inline void RunSortAndApplyOnAllElementsParallelDynamic() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_so_per_type = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};
  std::vector<int> batch_sizes = {std::max(1, num_threads - 1), num_threads, 3 * num_threads, 3 * num_threads + 1};

  for(auto n : num_so_per_type) {
    for(auto b : batch_sizes) {
      RunSortAndApplyOnAllElementsParallelDynamic<A, B>(n, b);
    }
  }

  for(auto b : batch_sizes) {
    RunSortAndApplyOnAllElementsParallelDynamic<A, B>(num_threads * 1000, b);
  }
}

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_
