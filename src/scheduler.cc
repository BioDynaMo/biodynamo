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

#include "scheduler.h"
#include "simulation.h"
#include "grid.h"

namespace bdm {

Scheduler::Scheduler() {
  auto* param = Simulation::GetActive()->GetParam();
  // backup_ = new SimulationBackup(param->backup_file_, param->restore_file_);
  // if (backup_->RestoreEnabled()) {
  //   restore_point_ = backup_->GetSimulationStepsFromBackup();
  // }
  // visualization_ =
  //     new CatalystAdaptor<>(BDM_SRC_DIR "/visualization/simple_pipeline.py");
}

Scheduler::~Scheduler() {
  // delete backup_;
  // delete visualization_;
  auto* param = Simulation::GetActive()->GetParam();
  if (param->statistics_) {
    std::cout << gStatistics << std::endl;
  }
}

void Scheduler::Simulate(uint64_t steps) {
  // if (Restore(&steps)) {
  //   return;
  // }

  Initialize();

  for (unsigned step = 0; step < steps; step++) {
    Execute(step == steps - 1);

    total_steps_++;
    // Backup();
  }
}

/// This function returns the numer of simulated steps (=iterations).
uint64_t Scheduler::GetSimulatedSteps() const { return total_steps_; }

void Scheduler::Execute(bool last_iteration) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* grid = sim->GetGrid();
  auto* param = sim->GetParam();

  assert(rm->GetNumSimObjects() > 0 &&
         "This simulation does not contain any simulation objects.");

  // visualization_->Visualize(total_steps_, last_iteration);

  {
    if (param->statistics_) {
      Timing timing("neighbors", &gStatistics);
      grid->UpdateGrid();
    } else {
      grid->UpdateGrid();
    }
  }
  // TODO(ahmad): should we only do it here and not after we run the physics?
  // We need it here, because we need to update the threshold values before
  // we update the diffusion grid
  if (param->bound_space_) {
    rm->ApplyOnAllElementsParallel(bound_space_);
  }
  // rm->ApplyOnAllTypes(diffusion_);
  rm->ApplyOnAllElementsParallel(biology_);
  if (param->run_mechanical_interactions_) {
    physics_->Init();
    rm->ApplyOnAllElementsParallel(physics_);  // Bounding box applied at the end
  }
  CommitChangesAndUpdateReferences();
}

void Scheduler::Backup() {
  // using std::chrono::seconds;
  // using std::chrono::duration_cast;
  // auto* param = Simulation::GetActive()->GetParam();
  // if (backup_->BackupEnabled() &&
  //     duration_cast<seconds>(Clock::now() - last_backup_).count() >=
  //         param->backup_interval_) {
  //   last_backup_ = Clock::now();
  //   backup_->Backup(total_steps_);
  // }
}

/// Restore the simulation if requested at the right time
/// @param steps number of simulation steps for a `Simulate` call
/// @return if `Simulate` should return early
bool Scheduler::Restore(uint64_t* steps) {
  // if (backup_->RestoreEnabled() && restore_point_ > total_steps_ + *steps) {
  //   total_steps_ += *steps;
  //   // restore requested, but not last backup was not done during this call to
  //   // Simualte. Therefore, we skip it.
  //   return true;
  // } else if (backup_->RestoreEnabled() && restore_point_ > total_steps_ &&
  //            restore_point_ < total_steps_ + *steps) {
  //   // Restore
  //   backup_->Restore();
  //   *steps = total_steps_ + *steps - restore_point_;
  //   total_steps_ = restore_point_;
  // }
  return false;
}

void Scheduler::CommitChangesAndUpdateReferences() {
//   auto* sim = Simulation::GetActive();
//   auto* rm = sim->GetResourceManager();
//   commit_->Reset();
//   rm->ApplyOnAllTypesParallel(commit_);
//
//   const auto& update_info = commit_->GetUpdateInfo();
//   auto update_references = [&update_info](auto* sim_objects,
//                                           uint16_t type_idx) {
// #pragma omp parallel for
//     for (uint64_t i = 0; i < sim_objects->size(); i++) {
//       (*sim_objects)[i].UpdateReferences(update_info);
//     }
//   };
//   rm->ApplyOnAllTypes(update_references);
}

// TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
// think about a better solution, because some operations are executed twice
// if Simulate is called with one timestep.
void Scheduler::Initialize() {
  CommitChangesAndUpdateReferences();

  auto* sim = Simulation::GetActive();
  auto* grid = sim->GetGrid();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();

  // if (!is_gpu_environment_initialized_ && param->use_gpu_) {
  //   InitializeGPUEnvironment<>();
  //   is_gpu_environment_initialized_ = true;
  // }

  grid->Initialize();
  if (param->bound_space_) {
    rm->ApplyOnAllElementsParallel(bound_space_);
  }
  int lbound = grid->GetDimensionThresholds()[0];
  int rbound = grid->GetDimensionThresholds()[1];
  // for (auto& dgrid : rm->GetDiffusionGrids()) {
  //   // Create data structures, whose size depend on the grid dimensions
  //   dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
  //   // Initialize data structures with user-defined values
  //   dgrid->RunInitializers();
  // }
}

}  // namespace bdm
