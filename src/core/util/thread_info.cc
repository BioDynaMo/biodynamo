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

#include <omp.h>
#include <sched.h>
#include <vector>

#include "core/util/log.h"
#include "core/util/numa.h"
#include "core/util/thread_info.h"

namespace bdm {

ThreadInfo* ThreadInfo::GetInstance() {
  static ThreadInfo kInstance;
  return &kInstance;
}

int ThreadInfo::GetMyThreadId() const { return omp_get_thread_num(); }

int ThreadInfo::GetMyNumaNode() const { return GetNumaNode(GetMyThreadId()); }

int ThreadInfo::GetMyNumaThreadId() const {
  return GetNumaThreadId(GetMyThreadId());
}

int ThreadInfo::GetNumaNodes() const { return numa_nodes_; }

int ThreadInfo::GetNumaNode(int omp_thread_id) const {
  return thread_numa_mapping_[omp_thread_id];
}

int ThreadInfo::GetThreadsInNumaNode(int numa_node) const {
  return threads_in_numa_[numa_node];
}

int ThreadInfo::GetNumaThreadId(int omp_thread_id) const {
  return numa_thread_id_[omp_thread_id];
}

int ThreadInfo::GetMaxThreads() const { return max_threads_; }

void ThreadInfo::Renew() {
  max_threads_ = omp_get_max_threads();
  numa_nodes_ = numa_num_configured_nodes();

  thread_numa_mapping_.clear();
  numa_thread_id_.clear();
  threads_in_numa_.clear();

  thread_numa_mapping_.resize(max_threads_, 0);
  numa_thread_id_.resize(max_threads_, 0);
  threads_in_numa_.resize(numa_nodes_, 0);

// (openmp thread id -> numa node)
#pragma omp parallel
  {
    int tid = omp_get_thread_num();
    thread_numa_mapping_[tid] = numa_node_of_cpu(sched_getcpu());
  }

  // (numa -> number of associated threads), and
  // (omp_thread_id -> thread id in numa)
  for (uint16_t n = 0; n < numa_nodes_; n++) {
    uint64_t cnt = 0;
    for (uint64_t t = 0; t < max_threads_; t++) {
      int numa = thread_numa_mapping_[t];
      if (n == numa) {
        numa_thread_id_[t] = cnt;
        cnt++;
      }
    }
    threads_in_numa_[n] = cnt;
  }
}

std::ostream& operator<<(std::ostream& str, const ThreadInfo& ti) {
  str << "max_threads            " << ti.max_threads_
      << "\nnum_numa nodes         " << ti.numa_nodes_;

  str << "\nthread to numa mapping ";
  for (auto& el : ti.thread_numa_mapping_) {
    str << " " << el;
  }

  str << "\nthread id in numa node ";
  for (auto& el : ti.numa_thread_id_) {
    str << " " << el;
  }

  str << "\nnum threads per numa   ";
  for (auto& el : ti.threads_in_numa_) {
    str << " " << el;
  }
  str << "\n";
  return str;
}

ThreadInfo::ThreadInfo() {
  auto proc_bind = omp_get_proc_bind();
  if (proc_bind != 1 && proc_bind != 4) {
    // 4 corresponds to OMP_PROC_BIND=spread
    // Due to some reason some OpenMP implementations set proc bind to spread
    // even though OMP_PROC_BIND is set to true.
    // A performance analysis showed almost identical results between true,
    // and spread.
    Log::Warning(
        "ThreadInfo::ThreadInfo",
        "The environment variable OMP_PROC_BIND must be set to "
        "true prior to running BioDynaMo ('export OMP_PROC_BIND=true')");
  }
  Renew();
}

}  // namespace bdm
