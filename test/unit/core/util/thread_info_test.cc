// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <set>

#include "core/util/thread_info.h"

namespace bdm {

void RunAllChecks(const ThreadInfo& ti) {
#ifdef BDM_USE_OMP
  EXPECT_EQ(omp_get_max_threads(), ti.GetMaxThreads());
#else
  EXPECT_EQ(1, ti.GetMaxThreads());
#endif  // BDM_USE_OMP
  EXPECT_EQ(numa_num_configured_nodes(), ti.GetNumaNodes());

  std::vector<int> threads_per_numa(ti.GetNumaNodes());
  std::vector<std::set<int>> all_numa_thread_ids(ti.GetNumaNodes());
#pragma omp parallel
  {
#pragma omp critical
    {
#ifdef BDM_USE_OMP
      int tid = omp_get_thread_num();
#else
      int tid = 0;
#endif  // BDM_USE_OMP
      EXPECT_EQ(tid, ti.GetMyThreadId());
      auto nid = numa_node_of_cpu(sched_getcpu());
      // check if mappting openmp thread id to numa node is correct
      EXPECT_EQ(nid, ti.GetNumaNode(tid));
      auto numa_thread_id = ti.GetNumaThreadId(tid);
      // numa thread id must be smaller than the max number of threads for this
      // numa node.
      EXPECT_LT(numa_thread_id, ti.GetThreadsInNumaNode(nid));
      std::set<int>& numa_thread_ids = all_numa_thread_ids[nid];
      // check if this numa_thread id has not been used before
      EXPECT_TRUE(numa_thread_ids.find(numa_thread_id) ==
                  numa_thread_ids.end());
      numa_thread_ids.insert(numa_thread_id);
      threads_per_numa[nid]++;
    }
  }

  for (uint16_t n = 0; n < ti.GetNumaNodes(); n++) {
    EXPECT_EQ(threads_per_numa[n], ti.GetThreadsInNumaNode(n));
  }
}

TEST(ThreadInfoTest, All) {
  auto* ti = ThreadInfo::GetInstance();
  RunAllChecks(*ti);
}

TEST(ThreadInfoTest, ThreadCPUBinding) {
  auto* ti = ThreadInfo::GetInstance();
  RunAllChecks(*ti);

  // do some work
  std::vector<int> v;
  v.resize(1e4);
#pragma omp parallel for
  for (uint64_t i = 0; i < v.size(); i++) {
    v[i]++;
  }

  // check if thread info is still correct
  RunAllChecks(*ti);
}

TEST(ThreadInfoTest, Renew) {
  auto* ti = ThreadInfo::GetInstance();
  RunAllChecks(*ti);

  // reduce number of threads to one
#ifdef BDM_USE_OMP
  auto omp_max_threads = omp_get_max_threads();
  omp_set_num_threads(1);
#endif  // BDM_USE_OMP
  ThreadInfo::GetInstance()->Renew();

  RunAllChecks(*ti);

  // Reset to previous condition
#ifdef BDM_USE_OMP
  omp_set_num_threads(omp_max_threads);
#endif  // BDM_USE_OMP
  ThreadInfo::GetInstance()->Renew();
}

}  // namespace bdm
