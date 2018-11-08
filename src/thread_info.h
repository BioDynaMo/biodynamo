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

// TODO rename to NumaInfo
// Documetnation be clear about thread id - rename in omp_thread_id
// and numa_thread_id

class ThreadInfo {
public:
  ThreadInfo() {
    if (omp_get_proc_bind() != 1) {
      // TODO throw exception / fatal
      Log::Fatal("ThreadInfo", "The environmental variable OMP_PROC_BIND must be set to true. On Linux run 'export OMP_PROC_BIND=true' prior to running BioDynaMo");
    }
    numa_nodes_ = numa_num_configured_nodes();
    thread_numa_mapping_.resize(omp_get_max_threads());
    numa_thread_id_.resize(omp_get_max_threads());
    threads_in_numa_.resize(numa_nodes_);

    Renew();
  }

  int GetNumaNodes() const { return numa_nodes_; }

  int GetNumaNode(int thread_id) const {
    return thread_numa_mapping_[thread_id];
  }

  int GetThreadsInNumaNode(int numa_node) const {
    return threads_in_numa_[numa_node];
  }

  int GetNumaThreadId(int thread_id) const {
    return numa_thread_id_[thread_id];
  }

  /// TODO document
  /// whenever a thread is scheduled on a different cpu e.g. using `numa_run_on_node` thread info needs to be updated
  void Renew() {
    // (thread id -> numa node)
    #pragma omp parallel
    {
      int tid = omp_get_thread_num();
      thread_numa_mapping_[tid] = numa_node_of_cpu(sched_getcpu());
    }

    // (numa -> number of associated threads), and
    // (thread_id -> thread id in numa)
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
  uint16_t numa_nodes_;

  /// Contains the mapping thread id -> numa node
  /// vector position = thread_id
  /// vector value = numa node
    std::vector<int> thread_numa_mapping_;

    /// Contains the mapping thread_id -> numa thread id
    /// each thread in a numa domain has a unique id in the range 0 to number of threads in this numa domain
    std::vector<int> numa_thread_id_;

    /// Contains the mapping numa node -> total number of threads in this numa node
    /// vector position: numa node
    /// vector value number of threads
    std::vector<int> threads_in_numa_;
};

}

#endif  // THREAD_INFO_H_
