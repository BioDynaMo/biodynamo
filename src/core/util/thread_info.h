// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_UTIL_THREAD_INFO_H_
#define CORE_UTIL_THREAD_INFO_H_

#include <omp.h>
#include <sched.h>
#include <atomic>
#include <vector>

#include "core/util/log.h"
#include "core/util/numa.h"

namespace bdm {

/// \brief This class stores information about each thread. (e.g. to which NUMA
/// node it belongs to.)
/// NB: Threads **must** be bound to CPUs using `OMP_PROC_BIND=true`.
class ThreadInfo {
 public:
  static ThreadInfo* GetInstance() {
    static ThreadInfo kInstance;
    return &kInstance;
  }

  // FIXME add test
  int GetMyThreadId() const { return omp_get_thread_num(); }

  // FIXME add test
  int GetMyNumaNode() const { return GetNumaNode(GetMyThreadId()); }

  /// Return the numa thread id of an openmp thread.
  int GetMyNumaThreadId() const { return GetNumaThreadId(GetMyThreadId()); }

  /// Returns the number of NUMA nodes on this machine
  int GetNumaNodes() const { return numa_nodes_; }

  /// Returns the numa node the given openmp thread is bound to.
  int GetNumaNode(int omp_thread_id) const {
    return thread_numa_mapping_[omp_thread_id];
  }

  /// Returns the number of threads in a given NUMA node.
  int GetThreadsInNumaNode(int numa_node) const {
    return threads_in_numa_[numa_node];
  }

  /// Return the numa thread id of an openmp thread.
  int GetNumaThreadId(int omp_thread_id) const {
    return numa_thread_id_[omp_thread_id];
  }

  /// Return the maximum number of threads.
  int GetMaxThreads() const { return max_threads_; }

  /// Returns a unique thread id even for parallel regions that
  /// don't use OpenMP.
  uint64_t GetUniversalThreadId() const;

  uint64_t GetMaxUniversalThreadId() const { return thread_counter_; }

  /// Renews the metadata.\n
  /// Whenever a thread is scheduled on a different cpu, e.g. using
  /// `numa_run_on_node`, `Renew()` must be called to update the thread
  /// metadata.
  void Renew() {
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

  friend std::ostream& operator<<(std::ostream& str, const ThreadInfo& ti) {
    str << "max_threads\t\t: " << ti.max_threads_
        << "\nnum_numa nodes\t\t: " << ti.numa_nodes_;

    str << "\nthread to numa mapping\t: ";
    for (auto& el : ti.thread_numa_mapping_) {
      str << el << " ";
    }

    str << "\nthread id in numa node\t: ";
    for (auto& el : ti.numa_thread_id_) {
      str << el << " ";
    }

    str << "\nnum threads per numa\t: ";
    for (auto& el : ti.threads_in_numa_) {
      str << el << " ";
    }
    str << "\n";
    return str;
  }

 private:
  static std::atomic<uint64_t> thread_counter_;

  /// Maximum number of threads for this simulation.
  uint64_t max_threads_;
  /// Number of NUMA nodes on this machine.
  uint16_t numa_nodes_;

  /// Contains the mapping thread id -> numa node \n
  /// vector position = omp_thread_id \n
  /// vector value = numa node
  std::vector<int> thread_numa_mapping_;

  /// Contains the mapping omp_thread_id -> numa thread id \n
  /// each thread in a numa domain has a unique id in the range 0 to number \n
  /// of threads in this numa domain
  std::vector<int> numa_thread_id_;

  /// Contains the mapping numa node -> total number of threads in this numa
  /// node \n
  /// vector position: numa node \n
  /// vector value number of threads
  std::vector<int> threads_in_numa_;

  ThreadInfo() {
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
};

}  // namespace bdm

#endif  // CORE_UTIL_THREAD_INFO_H_
