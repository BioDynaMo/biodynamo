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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <chrono>
#include <string>

#include "biology_module_op.h"
#include "bound_space_op.h"
#include "commit_op.h"
#include "diffusion_op.h"
#include "displacement_op.h"
#include "gpu/gpu_helper.h"
#include "op_timer.h"
#include "resource_manager.h"
#include "simulation_backup.h"

#include "log.h"
#include "param.h"
#include "simulation.h"
#include "visualization/catalyst_adaptor.h"

namespace bdm {

template <typename TSimulation = Simulation<>>
class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler() {
    auto* param = TSimulation::GetActive()->GetParam();
    backup_ = new SimulationBackup(param->backup_file_, param->restore_file_);
    if (backup_->RestoreEnabled()) {
      restore_point_ = backup_->GetSimulationStepsFromBackup();
    }
    visualization_ =
        new CatalystAdaptor<>(BDM_SRC_DIR "/visualization/simple_pipeline.py");
  }

  virtual ~Scheduler() {
    delete backup_;
    delete visualization_;
    auto* param = TSimulation::GetActive()->GetParam();
    if (param->statistics_) {
      std::cout << gStatistics << std::endl;
    }
  }

  void Simulate(uint64_t steps) {
    if (Restore(&steps)) {
      return;
    }

    Initialize();

    for (unsigned step = 0; step < steps; step++) {
      Execute(step == steps - 1);

      total_steps_++;
      Backup();
    }
  }

  /// This function returns the numer of simulated steps (=iterations).
  uint64_t GetSimulatedSteps() const { return total_steps_; }

 protected:
  uint64_t total_steps_ = 0;

  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute(bool last_iteration) {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto* grid = sim->GetGrid();
    auto* param = sim->GetParam();

    Timing::Time("visualize", [&]() {
      visualization_->Visualize(total_steps_, last_iteration);
    });
    Timing::Time("neighbors", [&]() { grid->UpdateGrid(); });
    // TODO(ahmad): should we only do it here and not after we run the physics?
    // We need it here, because we need to update the threshold values before
    // we update the diffusion grid
    if (param->bound_space_) {
      rm->ApplyOnAllTypes(bound_space_);
    }
    rm->ApplyOnAllTypes(biology_);
    if (param->run_mechanical_interactions_) {
      rm->ApplyOnAllTypes(physics_);  // Bounding box applied at the end
    }
    rm->ApplyOnAllElementsParallel(
        [](auto&& sim_object, SoHandle) { sim_object.RunDiscretization(); });
    CommitChangesAndUpdateReferences();
    Timing::Time("diffusion", diffusion_);
  }

 private:
  SimulationBackup* backup_ = nullptr;
  uint64_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  CatalystAdaptor<>* visualization_ = nullptr;  //!
  bool is_gpu_environment_initialized_ = false;

  OpTimer<CommitOp> commit_ = OpTimer<CommitOp>("commit");
  OpTimer<BiologyModuleOp> biology_ = OpTimer<BiologyModuleOp>("biology");
  OpTimer<DisplacementOp<TSimulation>> physics_ =
      OpTimer<DisplacementOp<TSimulation>>("physics");
  OpTimer<BoundSpace> bound_space_ = OpTimer<BoundSpace>("bound_space");
  DiffusionOp diffusion_;

  /// Backup the simulation. Backup interval based on `Param::backup_interval_`
  void Backup() {
    using std::chrono::seconds;
    using std::chrono::duration_cast;
    auto* param = TSimulation::GetActive()->GetParam();
    if (backup_->BackupEnabled() &&
        duration_cast<seconds>(Clock::now() - last_backup_).count() >=
            param->backup_interval_) {
      last_backup_ = Clock::now();
      backup_->Backup(total_steps_);
    }
  }

  /// Restore the simulation if requested at the right time
  /// @param steps number of simulation steps for a `Simulate` call
  /// @return if `Simulate` should return early
  bool Restore(uint64_t* steps) {
    if (backup_->RestoreEnabled() && restore_point_ > total_steps_ + *steps) {
      total_steps_ += *steps;
      // restore requested, but not last backup was not done during this call to
      // Simualte. Therefore, we skip it.
      return true;
    } else if (backup_->RestoreEnabled() && restore_point_ > total_steps_ &&
               restore_point_ < total_steps_ + *steps) {
      // Restore
      backup_->Restore();
      *steps = total_steps_ + *steps - restore_point_;
      total_steps_ = restore_point_;
    }
    return false;
  }

  void CommitChangesAndUpdateReferences() {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    commit_->Reset();
    rm->ApplyOnAllTypesParallel(commit_);

    const auto& update_info = commit_->GetUpdateInfo();
    auto update_references = [&update_info](auto* sim_objects,
                                            uint16_t type_idx) {
#pragma omp parallel for
      for (uint64_t i = 0; i < sim_objects->size(); i++) {
        (*sim_objects)[i].UpdateReferences(update_info);
      }
    };
    rm->ApplyOnAllTypes(update_references);
  }

  // TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
  // think about a better solution, because some operations are executed twice
  // if Simulate is called with one timestep.
  void Initialize() {
    CommitChangesAndUpdateReferences();

    auto* sim = TSimulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* rm = sim->GetResourceManager();
    auto* param = sim->GetParam();

    if (!is_gpu_environment_initialized_ && param->use_gpu_) {
      InitializeGPUEnvironment<>();
      is_gpu_environment_initialized_ = true;
    }

    if (param->bound_space_) {
      rm->ApplyOnAllTypes(bound_space_);
    }
    grid->Initialize();
    int lbound = grid->GetDimensionThresholds()[0];
    int rbound = grid->GetDimensionThresholds()[1];
    rm->ApplyOnAllDiffusionGrids([&](DiffusionGrid* dgrid) {
      // Create data structures, whose size depend on the grid dimensions
      dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
      // Initialize data structures with user-defined values
      dgrid->RunInitializers();
    });
  }
};

}  // namespace bdm

#endif  // SCHEDULER_H_
