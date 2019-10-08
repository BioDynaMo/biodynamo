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

#ifndef CORE_UTIL_NUMA_H_
#define CORE_UTIL_NUMA_H_

#ifdef USE_NUMA

#include <numa.h>

#else

#include <omp.h>

inline int numa_available() { return 0; }
inline int numa_num_configured_nodes() { return 1; }
inline int numa_num_configured_cpus() { return omp_get_max_threads(); }
inline int numa_run_on_node(int) { return 0; }
inline int numa_node_of_cpu(int) { return 0; }
inline int numa_move_pages(int pid, unsigned long count, void **pages,
                           const int *nodes, int *status, int flags) {
  *status = 0;
  return 0;
}

// on linux in <sched.h>, but missing on MacOS
inline int sched_getcpu() { return 0; }

#endif  // USE_NUMA

#endif  // CORE_UTIL_NUMA_H_
