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

#ifndef UNIT_CORE_RESOURCE_MANAGER_TEST_H_
#define UNIT_CORE_RESOURCE_MANAGER_TEST_H_

#include <algorithm>
#include <vector>
#include "core/grid.h"
#include "core/resource_manager.h"
#include "core/sim_object/sim_object.h"
#include "core/util/io.h"
#include "core/util/type.h"
#include "unit/test_util/test_sim_object.h"
#include "unit/test_util/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

class A : public TestSimObject {
  BDM_SIM_OBJECT_HEADER(A, TestSimObject, 1, data_);

 public:
  A() {}  // for ROOT I/O
  A(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}
  explicit A(int data) { data_ = data; }

  int GetData() const { return data_; }
  void SetData(int data) { data_ = data; }

  int data_;
};

class B : public TestSimObject {
  BDM_SIM_OBJECT_HEADER(B, TestSimObject, 1, data_);

 public:
  B() {}  // for ROOT I/O
  B(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}
  explicit B(double data) { data_ = data; }

  double GetData() const { return data_; }
  void SetData(double data) { data_ = data; }

  double data_;
};

inline void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  auto ref_uid = SoUidGenerator::Get()->GetLastId();
  ResourceManager rm;

  rm.push_back(new A(12));
  rm.push_back(new A(34));

  rm.push_back(new B(3.14));
  rm.push_back(new B(6.28));
  uint64_t counter = 0;
  rm.ApplyOnAllElements([&](SimObject* element) {  // NOLINT
    counter++;
    switch (element->GetUid() - ref_uid) {
      case 0:
        EXPECT_EQ(12, element->As<A>()->GetData());
        break;
      case 1:
        EXPECT_EQ(34, element->As<A>()->GetData());
        break;
      case 2:
        EXPECT_NEAR(3.14, element->As<B>()->GetData(), kEpsilon);
        break;
      case 3:
        EXPECT_NEAR(6.28, element->As<B>()->GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(4u, counter);
}

inline void RunGetNumSimObjects() {
  ResourceManager rm;

  rm.push_back(new A(12));
  rm.push_back(new A(34));
  rm.push_back(new A(59));

  rm.push_back(new B(3.14));
  rm.push_back(new B(6.28));

  EXPECT_EQ(5u, rm.GetNumSimObjects());
}

// This test uses Cells since SoaA, SoaB are strippted down simulatio objects
// and are themselves not thread safe.
inline void RunApplyOnAllElementsParallelTest() {
  ResourceManager rm;
  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  rm.push_back(new B(3.14));
  rm.push_back(new B(6.28));
  rm.push_back(new B(9.42));

  rm.ApplyOnAllElementsParallel([&](SimObject* sim_object) {
    const double kEpsilon = abs_error<double>::value;
    B* b = sim_object->As<B>();
    SoUid uid = sim_object->GetUid();
    if (uid == ref_uid) {
      EXPECT_EQ(3.14, b->GetData());
    } else if (uid == ref_uid + 1) {
      EXPECT_EQ(6.28, b->GetData());
    } else if (uid == ref_uid + 2) {
      EXPECT_NEAR(9.42, b->GetData(), kEpsilon);
    } else {
      FAIL();
    }
  });
}

inline void RunRemoveAndContainsTest() {
  ResourceManager rm;

  A* a0 = new A(12);
  auto a0_uid = a0->GetUid();
  rm.push_back(a0);

  A* a1 = new A(34);
  auto a1_uid = a1->GetUid();
  rm.push_back(a1);

  A* a2 = new A(59);
  auto a2_uid = a2->GetUid();
  rm.push_back(a2);

  B* b0 = new B(3.14);
  auto b0_uid = b0->GetUid();
  rm.push_back(b0);

  B* b1 = new B(6.28);
  auto b1_uid = b1->GetUid();
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

inline void RunClearTest() {
  ResourceManager rm;

  A* a0 = new A(12);
  auto a0_uid = a0->GetUid();
  rm.push_back(a0);

  A* a1 = new A(34);
  auto a1_uid = a1->GetUid();
  rm.push_back(a1);

  A* a2 = new A(59);
  auto a2_uid = a2->GetUid();
  rm.push_back(a2);

  B* b0 = new B(3.14);
  auto b0_uid = b0->GetUid();
  rm.push_back(b0);

  B* b1 = new B(6.28);
  auto b1_uid = b1->GetUid();
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

inline void RunPushBackAndGetSimObjectTest() {
  const double kEpsilon = abs_error<double>::value;
  auto ref_uid = SoUidGenerator::Get()->GetLastId();
  ResourceManager rm;

  rm.push_back(new A(12));
  rm.push_back(new A(34));

  rm.push_back(new B(3.14));
  rm.push_back(new B(6.28));

  rm.push_back(new A(87));

  EXPECT_EQ(rm.GetSimObject(ref_uid)->As<A>()->GetData(), 12);
  EXPECT_EQ(rm.GetSimObject(ref_uid + 1)->As<A>()->GetData(), 34);
  EXPECT_EQ(rm.GetSimObject(ref_uid + 4)->As<A>()->GetData(), 87);

  EXPECT_NEAR(rm.GetSimObject(ref_uid + 2)->As<B>()->GetData(), 3.14, kEpsilon);
  EXPECT_NEAR(rm.GetSimObject(ref_uid + 3)->As<B>()->GetData(), 6.28, kEpsilon);
}

// -----------------------------------------------------------------------------
// https://github.com/osmhpi/pgasus/blob/775a5f90d8f6fa89cfb93eac6de16dcfe27167ce/src/util/mmaphelper.cpp
inline static void* AlignPage(const void* ptr) {
  static constexpr uintptr_t kPageMask = ~(uintptr_t(0xFFF));
  return (void*)(((uintptr_t)ptr) & kPageMask);
}

inline int GetNumaNodeForMemory(const void* ptr) {
  int result, loc;
  void* pptr = AlignPage(ptr);
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
inline void CheckApplyOnAllElements(ResourceManager* rm,
                                    uint64_t num_so_per_type,
                                    bool numa_checks = false) {
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

  rm->ApplyOnAllElementsParallel([&](SimObject* so) {
    size_t index = 0;
    if (A* a = so->As<A>()) {
      index = a->GetData();
    } else if (B* b = so->As<B>()) {
      index = std::round(b->GetData());
    }
    auto handle = rm->GetSoHandle(so->GetUid());

#pragma omp critical
    {
      found[index] = true;

      // verify that a thread processes sim objects on the same NUMA node.
      if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(so)) {
        numa_memory_errors++;
      }
      if (numa_checks &&
          handle.GetNumaNode() != numa_node_of_cpu(sched_getcpu())) {
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
      FAIL()
          << "ApplyOnAllElementsParallel was not called for element with data_="
          << i;
    }
  }

  if (numa_checks) {
    EXPECT_EQ(0u, numa_memory_errors.load());
    EXPECT_EQ(0u, numa_thread_errors.load());
    auto so_per_numa = GetSoPerNuma(2 * num_so_per_type);
    for (int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

inline void RunSortAndApplyOnAllElementsParallel(uint64_t num_so_per_type) {
  Simulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<SoUid, double> a_x_values;
  std::unordered_map<SoUid, double> b_x_values;
  for (uint64_t i = 0; i < num_so_per_type; ++i) {
    double x_pos = i * 30.0;

    A* a = new A(i);
    a->SetDiameter(10);
    a->SetPosition({x_pos, 0, 0});
    rm->push_back(a);
    a_x_values[a->GetUid()] = x_pos;

    B* b = new B(i + num_so_per_type);
    b->SetDiameter(10);
    b->SetPosition({x_pos, 0, 0});
    rm->push_back(b);
    b_x_values[b->GetUid()] = x_pos;
  }

  CheckApplyOnAllElements(rm, num_so_per_type);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElements(rm, num_so_per_type, true);

  // check if sim object uids still point to the correct object
  for (auto& entry : a_x_values) {
    auto x_actual = rm->GetSimObject(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for (auto& entry : b_x_values) {
    auto x_actual = rm->GetSimObject(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

inline void RunSortAndApplyOnAllElementsParallel() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_so_per_type = {std::max(1, num_threads - 1), num_threads,
                                      3 * num_threads, 3 * num_threads + 1};

  for (auto n : num_so_per_type) {
    RunSortAndApplyOnAllElementsParallel(n);
  }

  RunSortAndApplyOnAllElementsParallel(1000);
}

// //
// -----------------------------------------------------------------------------
inline void CheckApplyOnAllElementsDynamic(ResourceManager* rm,
                                           uint64_t num_so_per_type,
                                           uint64_t batch_size,
                                           bool numa_checks = false) {
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

  rm->ApplyOnAllElementsParallelDynamic(
      batch_size, [&](SimObject* so, SoHandle handle) {
#pragma omp critical
        {
          size_t index = 0;
          if (A* a = so->As<A>()) {
            index = a->GetData();
          } else if (B* b = so->As<B>()) {
            index = std::round(b->GetData());
          }
          found[index] = true;

          // verify that a thread processes sim objects on the same NUMA node.
          if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(so)) {
            numa_memory_errors++;
          }

          numa_so_cnts[handle.GetNumaNode()]++;
        }
        cnt++;
      });

  // critical sections increase the variance of numa_thread_errors.
  // Therefore, there are checked separately.
  rm->ApplyOnAllElementsParallelDynamic(
      batch_size, [&](SimObject* so, SoHandle handle) {
        volatile double d = 0;
        for (int i = 0; i < 10000; i++) {
          d += std::sin(i);
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
      FAIL()
          << "ApplyOnAllElementsParallel was not called for element with data_="
          << i;
    }
  }

  if (numa_checks) {
    // If there are memory errors, check of
    // `cat /proc/sys/kernel/numa_balancing` is zero.
    // Automatic rebalancing can lead to numa memory errors.
    // only 0.1% of all sim objects may be on a wrong numa node
    EXPECT_GT(0.001, (numa_memory_errors.load() + 0.0) / (2 * num_so_per_type));
    // work stealing can cause thread errors. This check ensures that at least
    // 75% of the work is done by the correct CPU-Memory mapping.
    if (num_so_per_type > 20 * static_cast<uint64_t>(omp_get_max_threads())) {
      EXPECT_GT(num_so_per_type / 4, numa_thread_errors.load());
    }
    auto so_per_numa = GetSoPerNuma(2 * num_so_per_type);
    for (int n = 0; n < ti.GetNumaNodes(); ++n) {
      EXPECT_EQ(so_per_numa[n], numa_so_cnts[n]);
    }
  }
}

inline void RunSortAndApplyOnAllElementsParallelDynamic(
    uint64_t num_so_per_type, uint64_t batch_size) {
  Simulation simulation("RunSortAndApplyOnAllElementsParallel");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<SoUid, double> a_x_values;
  std::unordered_map<SoUid, double> b_x_values;
  for (uint64_t i = 0; i < num_so_per_type; ++i) {
    double x_pos = i * 30.0;

    A* a = new A(i);
    a->SetDiameter(10);
    a->SetPosition({x_pos, 0, 0});
    rm->push_back(a);
    a_x_values[a->GetUid()] = x_pos;

    B* b = new B(i + num_so_per_type);
    b->SetDiameter(10);
    b->SetPosition({x_pos, 0, 0});
    rm->push_back(b);
    b_x_values[b->GetUid()] = x_pos;
  }

  CheckApplyOnAllElementsDynamic(rm, num_so_per_type, batch_size);

  simulation.GetGrid()->UpdateGrid();
  rm->SortAndBalanceNumaNodes();

  CheckApplyOnAllElementsDynamic(rm, num_so_per_type, batch_size, true);

  // check if sim object uids still point to the correct object
  for (auto& entry : a_x_values) {
    auto x_actual = rm->GetSimObject(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for (auto& entry : b_x_values) {
    auto x_actual = rm->GetSimObject(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

inline void RunSortAndApplyOnAllElementsParallelDynamic() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_so_per_type = {std::max(1, num_threads - 1), num_threads,
                                      3 * num_threads, 3 * num_threads + 1};
  std::vector<int> batch_sizes = {std::max(1, num_threads - 1), num_threads,
                                  3 * num_threads, 3 * num_threads + 1};

  for (auto n : num_so_per_type) {
    for (auto b : batch_sizes) {
      RunSortAndApplyOnAllElementsParallelDynamic(n, b);
    }
  }

  for (auto b : batch_sizes) {
    RunSortAndApplyOnAllElementsParallelDynamic(num_threads * 1000, b);
  }
}

inline void RunIOTest() {
  const double kEpsilon = abs_error<double>::value;
  auto ref_uid = SoUidGenerator::Get()->GetLastId();
  ResourceManager rm;
  remove(ROOTFILE);

  // setup
  rm.push_back(new A(12));
  rm.push_back(new A(34));
  rm.push_back(new A(42));

  rm.push_back(new B(3.14));
  rm.push_back(new B(6.28));

  DiffusionGrid* dgrid_1 = new DiffusionGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid(1, "Natrium", 0.2, 0.1, 1);
  rm.AddDiffusionGrid(dgrid_1);
  rm.AddDiffusionGrid(dgrid_2);

  // backup
  WritePersistentObject(ROOTFILE, "rm", rm, "new");

  rm.Clear();

  // restore
  ResourceManager* restored_rm = nullptr;
  GetPersistentObject(ROOTFILE, "rm", restored_rm);
  restored_rm->RestoreUidSoMap();

  // validate
  EXPECT_EQ(5u, restored_rm->GetNumSimObjects());

  EXPECT_EQ(12, restored_rm->GetSimObject(ref_uid)->As<A>()->GetData());
  EXPECT_EQ(34, restored_rm->GetSimObject(ref_uid + 1)->As<A>()->GetData());
  EXPECT_EQ(42, restored_rm->GetSimObject(ref_uid + 2)->As<A>()->GetData());

  EXPECT_NEAR(3.14, restored_rm->GetSimObject(ref_uid + 3)->As<B>()->GetData(),
              kEpsilon);
  EXPECT_NEAR(6.28, restored_rm->GetSimObject(ref_uid + 4)->As<B>()->GetData(),
              kEpsilon);

  EXPECT_EQ(0, restored_rm->GetDiffusionGrid(0)->GetSubstanceId());
  EXPECT_EQ(1, restored_rm->GetDiffusionGrid(1)->GetSubstanceId());
  EXPECT_EQ("Kalium", restored_rm->GetDiffusionGrid(0)->GetSubstanceName());
  EXPECT_EQ("Natrium", restored_rm->GetDiffusionGrid(1)->GetSubstanceName());
  EXPECT_EQ(0.6,
            restored_rm->GetDiffusionGrid(0)->GetDiffusionCoefficients()[0]);
  EXPECT_EQ(0.8,
            restored_rm->GetDiffusionGrid(1)->GetDiffusionCoefficients()[0]);

  delete restored_rm;

  remove(ROOTFILE);
}

}  // namespace bdm

#endif  // UNIT_CORE_RESOURCE_MANAGER_TEST_H_
