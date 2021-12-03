// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifdef USE_MFEM
#include "core/operation/visualization_mfem_op.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "mfem.hpp"

namespace bdm {

VisualizationMFEMOp::~VisualizationMFEMOp() {
  // Free memory, each *pv_data was dynamically allocated on the heap.
  for (auto* pv_data : paraview_data_) {
    delete pv_data;
  }
}

void VisualizationMFEMOp::Initialize() {
  // Define output directory as <bdm-output-dir>/MFEM_Continuum
  auto output_dir = Simulation::GetActive()->GetOutputDir() + "/MFEM_Continuum";

  // Create one ParaViewDataCollection for each MFEM continuum registered in the
  // resource manager.
  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (size_t grid_id = 0; grid_id < rm->GetNumMFEMMeshes(); grid_id++) {
    // Get pointers and relevant data from ResourceManger
    auto mfem_grid = rm->GetMFEMGrid(grid_id);
    auto* mesh = mfem_grid.first;
    auto* field = mfem_grid.second;
    auto name_of_quantity = field->GetSubstanceName();

    // Create a ParaViewDataCollection
    mfem::ParaViewDataCollection* pv_data =
        new mfem::ParaViewDataCollection(name_of_quantity, mesh);
    pv_data->SetPrefixPath(output_dir);
    pv_data->SetLevelsOfDetail(field->GetPolynomialDegree());
    pv_data->SetDataFormat(mfem::VTKFormat::BINARY);
    pv_data->SetCycle(0);
    pv_data->SetTime(0.0);
    pv_data->RegisterField(name_of_quantity, field->GetGridFunction());
    pv_data->Save();
    paraview_data_.push_back(pv_data);
  }
  // Set initialized_ to true so that the function is not called a second time
  // when the operator() is accessed.
  initialized_ = true;
}

void VisualizationMFEMOp::operator()() {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto* param = Simulation::GetActive()->GetParam();
  // The option insitu_visualization is not supported
  if (param->insitu_visualization == true) {
    if (!has_warned_) {
      Log::Warning("VisualizationMFEMOp",
                   "Insitu Visualization is not supported for MFEM.\n",
                   "Please use the parameter export_visualization=true.");
      has_warned_ = true;
    }
    return;
  }
  // If there is no MFEM continuum or the user does not want to export the
  // data, we return immediately.
  if (rm->GetNumMFEMMeshes() == 0 || param->export_visualization == false) {
    return;
  }
  // The operation is always called but the visualization frequency is steered
  // with the parameter visualization_interval. Because the visualization of the
  // DiffusionGrid falls back to the ParaViewAdaptor handling the frequency, we
  // use the same approach here rather than using the frequency member.
  auto* scheduler = Simulation::GetActive()->GetScheduler();
  int simulated_steps = static_cast<int>(scheduler->GetSimulatedSteps());
  double simulated_time = scheduler->GetSimulatedTime();
  if (simulated_steps % param->visualization_interval != 0) {
    return;
  }

  // Save output
  if (!initialized_) {
    // Initialize when operator() is called for the first time
    Initialize();
  } else {
    // Save the output for each MFEM continuum at the particular time step
    for (auto* pv_data : paraview_data_) {
      pv_data->SetCycle(simulated_steps);
      pv_data->SetTime(simulated_time);
      pv_data->Save();
    }
  }
}

}  // namespace bdm

#endif  // USE_MFEM