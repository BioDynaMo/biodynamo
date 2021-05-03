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

#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>

#include "core/visualization/root/root_eve_adaptor.h"
#include <TTree.h>

namespace bdm {

// ----------------------------------------------------------------------------
struct RootEveAdaptor::RootEveImpl {
  TTree *pointsTree_ = nullptr;
};

// ----------------------------------------------------------------------------
std::atomic<uint64_t> RootEveAdaptor::counter_;

// ----------------------------------------------------------------------------
RootEveAdaptor::RootEveAdaptor() {
  counter_++;
  impl_ = std::unique_ptr<RootEveAdaptor::RootEveImpl>(
      new RootEveAdaptor::RootEveImpl());
}

// ----------------------------------------------------------------------------
RootEveAdaptor::~RootEveAdaptor() {
  auto* param = Simulation::GetActive()->GetParam();
  counter_--;

  if (impl_) {
    if (param->export_visualization &&
        param->visualization_export_generate_tree) {
      WriteSimulationInfo();
      WritePointsTree();
    }
  }
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::Visualize() {
  if (!initialized_) {
    Initialize();
    initialized_ = true;
  }

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  uint64_t total_steps = sim->GetScheduler()->GetSimulatedSteps();
  if (total_steps % param->visualization_interval != 0) {
    return;
  }
  
  double time = param->simulation_time_step * total_steps;

  printf("**** Create TEvePoints entry time = %g ****\n", time);

  if (param->insitu_visualization) {
    InsituVisualization();
  }
  if (param->export_visualization) {
    ExportVisualization();
  }
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::Initialize() {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();

  if (param->insitu_visualization) {
    Log::Warning("RootEveAdaptor",
                 "Insitu visualization is currently not supported. "
                 "Please use export visualization.");
  }
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::InsituVisualization() {
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::ExportVisualization() {
  
  // loop over all agents and write positions and state
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::BuildAgentsRootStructures() {
  auto* rm = Simulation::GetActive()->GetResourceManager();
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::WritePointsTree() {
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::WriteSimulationInfo() {
}

}  // namespace bdm
