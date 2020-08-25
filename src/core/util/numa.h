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

#include <cstdint>

int numa_available();
int numa_num_configured_nodes();
int numa_num_configured_cpus();
int numa_run_on_node(int);
int numa_node_of_cpu(int);
int numa_move_pages(int pid, unsigned long count, void **pages,
                    const int *nodes, int *status, int flags);
void *numa_alloc_onnode(uint64_t size, int nid);
void numa_free(void *p, uint64_t);

// on linux in <sched.h>, but missing on MacOS
int sched_getcpu();

#endif  // USE_NUMA

#endif  // CORE_UTIL_NUMA_H_
