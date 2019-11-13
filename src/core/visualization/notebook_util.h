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
#ifndef CORE_VISUALIZATION_NOTEBOOK_UTIL
#define CORE_VISUALIZATION_NOTEBOOK_UTIL

#include "core/simulation.h"
#include "core/visualization/root_adaptor.h"

namespace bdm {

/// Visualize the simulation objects in ROOT notebooks
inline void VisualizeInNotebook(size_t w = 300, size_t h = 300,
                                std::string opt = "") {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  // Force an update of the visualization engine
  sim->GetScheduler()->GetRootVisualization()->Visualize(
      param->visualization_export_interval_);
  sim->GetScheduler()->GetRootVisualization()->DrawInCanvas(w, h, opt);
}

}  // namespace bdm

#endif  // CORE_VISUALIZATION_NOTEBOOK_UTIL
