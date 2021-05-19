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


#include "core/visualization/rooteve/root_eve_adaptor.h"

namespace bdm {

// ----------------------------------------------------------------------------
struct RootEveAdaptor::RootEveImpl {
  Float_t time;
  Int_t   nagents;
  Float_t *x = nullptr;
  Float_t *y = nullptr;
  Float_t *z = nullptr;
  Float_t *d = nullptr;
  //std::vector<void*>void   **br = nullptr;   // array of arrays of additional attributes

  RootEveImpl(int max_agents = 0);
  ~RootEveImpl();
};

RootEveAdaptor::RootEveImpl::RootEveImpl(Int_t max_agents) {
  time    = 0.;
  nagents = 0;
  x = new Float_t [max_agents];
  y = new Float_t [max_agents];
  z = new Float_t [max_agents];
  d = new Float_t [max_agents];

  printf("### Create RootEveImpl arrays of size %d\n", max_agents);
}

RootEveAdaptor::RootEveImpl::~RootEveImpl() {
  delete [] x;
  delete [] y;
  delete [] z;
  delete [] d;

  printf("### Delete RootEveImpl arrays\n");
}

// ----------------------------------------------------------------------------
RootEveAdaptor::RootEveAdaptor() : initialized_(false), max_vis_agents_(1e6) {
  impl_ = std::unique_ptr<RootEveAdaptor::RootEveImpl>(
            new RootEveAdaptor::RootEveImpl(max_vis_agents_));
}

// ----------------------------------------------------------------------------
RootEveAdaptor::~RootEveAdaptor() {
  auto* param = Simulation::GetActive()->GetParam();

  if (impl_) {
    if (param->export_visualization &&
        param->visualization_export_generate_tree) {
      WriteSimulationInfo();
      WriteTree();
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

  vis_time_ = param->simulation_time_step * total_steps;

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

  if (param->export_visualization) {
    outfile_ = param->output_dir + "/";
    outfile_ += sim->GetUniqueName() + ".root";

    // open ROOT file to store the tree
    file_ = TFile::Open(outfile_, "RECREATE");

    // create the tree with agent positions
    tree_ = new TTree("bdmvis", "Agent attributes for visualization");
    tree_->Branch("time", &impl_->time, "time/F");
    tree_->Branch("nagents", &impl_->nagents, "nagents/I");
    tree_->Branch("x", impl_->x, "x[nagents]/F");
    tree_->Branch("y", impl_->y, "y[nagents]/F");
    tree_->Branch("z", impl_->z, "z[nagents]/F");
    tree_->Branch("d", impl_->d, "d[nagents]/F");
    //tree_->Branch("state", impl_->state, "state[nagents]/I");
    // add extra branches depending on what is defined in the param's
  }

  initialized_ = true;
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::InsituVisualization() {
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::ExportVisualization() {
  auto* sim = Simulation::GetActive();
  auto *rm = sim->GetResourceManager();

  impl_->time = static_cast<Float_t>(vis_time_);
  impl_->nagents = static_cast<Int_t>(rm->GetNumAgents());
printf("**** time = %f, nagents = %d ****\n", impl_->time, impl_->nagents);
  rm->ForEachAgent([&](Agent *agent) {
    auto idx = agent->GetUid().GetIndex();
    impl_->x[idx] = static_cast<Float_t>(agent->GetPosition()[0]);
    impl_->y[idx] = static_cast<Float_t>(agent->GetPosition()[1]);
    impl_->z[idx] = static_cast<Float_t>(agent->GetPosition()[2]);
    impl_->d[idx] = static_cast<Float_t>(agent->GetDiameter());
  });

  tree_->Fill();
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::WriteTree() {
  if (initialized_) {
    tree_->Print();
    tree_->Write();
    file_->Close();
  }
}

// ----------------------------------------------------------------------------
void RootEveAdaptor::WriteSimulationInfo() {
}

}  // namespace bdm
