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

#ifndef THREAD_INFO_H_
#define THREAD_INFO_H_

#include <sched.h>
#include <numa.h>
#include <omp.h>

#include "log.h"

namespace bdm {

/// \brief This class stores information about each thread. (e.g. to which NUMA
/// node it belongs to.)
/// NB: Threads **must** be bound to CPUs using `OMP_PROC_BIND=true`.
class ThreadInfo {
public:
  ThreadInfo() {
    if (omp_get_proc_bind() != 1) {
      Log::Fatal("ThreadInfo", "The environmental variable OMP_PROC_BIND must be set to true. On Linux run 'export OMP_PROC_BIND=true' prior to running BioDynaMo");
    }
    max_threads_ = omp_get_max_threads();
    numa_nodes_ = numa_num_configured_nodes();
    thread_numa_mapping_.resize(max_threads_);
    numa_thread_id_.resize(max_threads_);
    threads_in_numa_.resize(numa_nodes_);
    Renew();
  }

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

  /// Renews the metadata.\n
  /// Whenever a thread is scheduled on a different cpu, e.g. using
  /// `numa_run_on_node`, `Renew()` must be called to update the thread
  /// metadata.
  void Renew() {
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
      for(uint64_t t = 0; t < thread_numa_mapping_.size(); t++) {
        int numa = thread_numa_mapping_[t];
        if (n == numa) {
          numa_thread_id_[t] = cnt;
          cnt++;
        }
      }
      threads_in_numa_[n] = cnt;
    }
  }

private:
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
};

}

#endif  // THREAD_INFO_H_
