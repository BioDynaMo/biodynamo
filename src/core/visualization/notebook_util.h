#ifndef CORE_VISUALIZATION_NOTEBOOK_UTIL
#define CORE_VISUALIZATION_NOTEBOOK_UTIL

#include "core/simulation.h"
#include "core/visualization/root_adaptor.h"

namespace bdm {

/// Visualize the simulation objects in ROOT notebooks
void VisualizeInNotebook(size_t w = 300, size_t h = 300, std::string opt = "") {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  // Force an update of the visualization engine
  sim->GetScheduler()->GetRootVisualization()->Visualize(
      param->visualization_export_interval_);
  sim->GetScheduler()->GetRootVisualization()->DrawInCanvas(w, h, opt);
}

}  // namespace bdm

#endif  // CORE_VISUALIZATION_NOTEBOOK_UTIL