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

#ifndef CORE_UTIL_THREAD_INFO_H_
#define CORE_UTIL_THREAD_INFO_H_

#include <vector>

namespace bdm {

/// \brief This class stores information about each thread. (e.g. to which NUMA
/// node it belongs to.)
/// NB: Threads **must** be bound to CPUs using `OMP_PROC_BIND=true`.
class ThreadInfo {
 public:
  static ThreadInfo* GetInstance();

  // FIXME add test
  int GetMyThreadId() const;

  // FIXME add test
  int GetMyNumaNode() const;

  /// Return the numa thread id of an openmp thread.
  int GetMyNumaThreadId() const;

  /// Returns the number of NUMA nodes on this machine
  int GetNumaNodes() const;

  /// Returns the numa node the given openmp thread is bound to.
  int GetNumaNode(int omp_thread_id) const;

  /// Returns the number of threads in a given NUMA node.
  int GetThreadsInNumaNode(int numa_node) const;

  /// Return the numa thread id of an openmp thread.
  int GetNumaThreadId(int omp_thread_id) const;

  /// Return the maximum number of threads.
  int GetMaxThreads() const;

  /// Renews the metadata.\n
  /// Whenever a thread is scheduled on a different cpu, e.g. using
  /// `numa_run_on_node`, `Renew()` must be called to update the thread
  /// metadata.
  void Renew();

  friend std::ostream& operator<<(std::ostream& str, const ThreadInfo& ti);

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

  ThreadInfo();
};

}  // namespace bdm

#endif  // CORE_UTIL_THREAD_INFO_H_
