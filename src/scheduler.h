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

#include "bound_space_op.h"
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

    Timing::Time("Set up exec context", [&]() {
      for (auto* ctxt : sim->GetAllExecCtxts()) {
        ctxt->SetupIteration();
      }
    });

    Timing::Time("visualize", [&]() {
      visualization_->Visualize(total_steps_, last_iteration);
    });
    Timing::Time("neighbors", [&]() { grid->UpdateGrid(); });

    // create ops
    auto bound_space_op = [&](auto&& so) {
      if (param->bound_space_) {
        ApplyBoundingBox(&so, param->min_bound_, param->max_bound_);
      }
    };

    auto biology_module_op = [&](auto&& so) { so.RunBiologyModules(); };
    auto discretization_op = [&](auto&& so) { so.RunDiscretization(); };

    auto displacement_op = [&](auto&& so) {
      if (param->run_mechanical_interactions_ && displacement_.UseCpu()) {
        displacement_(so);
      }
    };

    // update all sim objects: run all CPU operations
    rm->ApplyOnAllTypes([&](auto* sim_objects, uint16_t type_idx) {
#pragma omp parallel for schedule(dynamic, 100)
      for (size_t i = 0; i < sim_objects->size(); i++) {
        auto&& so = (*sim_objects)[i];
        sim->GetExecutionContext()->Execute(so, bound_space_op,
                                            biology_module_op, displacement_op,
                                            discretization_op);
      }
    });

    // update all sim objects: hardware accelerated operations
    if (param->run_mechanical_interactions_ && !displacement_.UseCpu()) {
      Timing::Time("displacement (GPU/FPGA)", displacement_);
    }

    // finish updating sim objects
    Timing::Time("Tear down exec context", [&]() {
      for (auto* ctxt : sim->GetAllExecCtxts()) {
        ctxt->TearDownIteration();
      }
    });

    // update all substances (DiffusionGrids)
    Timing::Time("diffusion", diffusion_);
  }

 private:
  SimulationBackup* backup_ = nullptr;
  uint64_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  CatalystAdaptor<>* visualization_ = nullptr;  //!
  bool is_gpu_environment_initialized_ = false;

  OpTimer<BoundSpace> bound_space_ = OpTimer<BoundSpace>("bound_space");
  DisplacementOp<> displacement_;
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

  // TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
  // think about a better solution, because some operations are executed twice
  // if Simulate is called with one timestep.
  void Initialize() {
    auto* sim = TSimulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* rm = sim->GetResourceManager();
    auto* param = sim->GetParam();

    // commit all changes
    for (auto* ctxt : sim->GetAllExecCtxts()) {
      ctxt->TearDownIteration();
    }

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
