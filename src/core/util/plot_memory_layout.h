// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_UTIL_PLOT_MEMORY_LAYOUT_H_
#define CORE_UTIL_PLOT_MEMORY_LAYOUT_H_

#include <vector>

namespace bdm {

class Agent;

// -----------------------------------------------------------------------------
void PlotMemoryLayout(const std::vector<Agent*>& agents, int numa_node);

// -----------------------------------------------------------------------------
void PlotMemoryHistogram(const std::vector<Agent*>& agents, int numa_node);

// -----------------------------------------------------------------------------
void PlotNeighborMemoryHistogram(bool before = false);

}  // namespace bdm

#endif  // CORE_UTIL_PLOT_MEMORY_LAYOUT_H_
