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

#include "core/scheduler.h"

#include <chrono>
#include <string>

#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/gpu/gpu_helper.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/diffusion_op.h"
#include "core/operation/displacement_op.h"
#include "core/operation/op_timer.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/simulation_backup.h"
#include "core/util/log.h"
#include "core/visualization/catalyst_adaptor.h"
#include "gui/controller/visualization_manager.h"

namespace bdm {

Scheduler::Scheduler() {
  auto* param = Simulation::GetActive()->GetParam();
  backup_ = new SimulationBackup(param->backup_file_, param->restore_file_);
  if (backup_->RestoreEnabled()) {
    restore_point_ = backup_->GetSimulationStepsFromBackup();
  }
  visualization_ =
      new CatalystAdaptor(BDM_SRC_DIR "/visualization/simple_pipeline.py");
  bound_space_ = new BoundSpace();
  displacement_ = new DisplacementOp();
  diffusion_ = new DiffusionOp();

  is_gui_enabled = gui::VisManager::GetInstance().IsEnabled();

  // initialise operations_
  auto first_op =
      Operation("first", [](SimObject* so) { so->UpdateRunDisplacement(); });
  auto last_op = Operation(
      "last", [](SimObject* so) { so->ApplyRunDisplacementForAllNextTs(); });
  auto biology_module_op = Operation(
      "biology modules", [](SimObject* so) { so->RunBiologyModules(); });
  auto discretization_op = Operation(
      "discretization", [](SimObject* so) { so->RunDiscretization(); });

  auto displacement_op = Operation("displacement", *displacement_);

  operations_ = {first_op,          Operation("bound space", *bound_space_),
                 biology_module_op, displacement_op,
                 discretization_op, last_op};
  protected_operations_ = {first_op.name_, biology_module_op.name_,
                           discretization_op.name_, last_op.name_};
}

Scheduler::~Scheduler() {
  delete backup_;
  delete visualization_;
  delete bound_space_;
  delete displacement_;
  delete diffusion_;
  auto* param = Simulation::GetActive()->GetParam();
  if (param->statistics_) {
    std::cout << gStatistics << std::endl;
  }
}

void Scheduler::Simulate(uint64_t steps) {
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

uint64_t Scheduler::GetSimulatedSteps() const { return total_steps_; }

void Scheduler::AddOperation(const Operation& op) {
  auto it = operations_.end() - 2;
  operations_.insert(it, op);
}

void Scheduler::RemoveOperation(const std::string& name) {
  if (protected_operations_.find(name) != protected_operations_.end()) {
    Log::Warning("Scheduler::GetOperation",
                 "You tried to remove the protected operation ", name,
                 "! This request was ignored.");
    return;
  }
  for (auto it = operations_.begin(); it != operations_.end(); ++it) {
    if (name == it->name_) {
      operations_.erase(it);
      return;
    }
  }
}

Operation* Scheduler::GetOperation(const std::string& name) {
  if (protected_operations_.find(name) != protected_operations_.end()) {
    Log::Warning("Scheduler::GetOperation",
                 "You tried to get the protected operation ", name, "!");
    return nullptr;
  }
  for (uint64_t i = 0; i < operations_.size(); i++) {
    auto& op = operations_[i];
    if (name == op.name_) {
      return &op;
    }
  }
  return nullptr;
}

void Scheduler::Execute(bool last_iteration) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* grid = sim->GetGrid();
  auto* param = sim->GetParam();

  Timing::Time("Set up exec context", [&]() {
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
  });

  Timing::Time("visualize", [&]() {
    visualization_->Visualize(total_steps_, last_iteration);
  });

  if(is_gui_enabled) {
    gui::VisManager::GetInstance().Update();
  }
  Timing::Time("neighbors", [&]() { grid->UpdateGrid(); });

  // update all sim objects: run all CPU operations
  const auto& scheduled_ops = GetScheduleOps();
  rm->ApplyOnAllElementsParallelDynamic(
      param->scheduling_batch_size_, [&](SimObject* so, SoHandle) {
        sim->GetExecutionContext()->Execute(so, scheduled_ops);
      });

  // update all sim objects: hardware accelerated operations
  if (param->run_mechanical_interactions_ && !displacement_->UseCpu()) {
    Timing::Time("displacement (GPU/FPGA)", *displacement_);
  }

  // finish updating sim objects
  Timing::Time("Tear down exec context", [&]() {
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->TearDownIterationAll(all_exec_ctxts);
  });

  // update all substances (DiffusionGrids)
  Timing::Time("diffusion", *diffusion_);
}

void Scheduler::Backup() {
  using std::chrono::seconds;
  using std::chrono::duration_cast;
  auto* param = Simulation::GetActive()->GetParam();
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
bool Scheduler::Restore(uint64_t* steps) {
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
void Scheduler::Initialize() {
  auto* sim = Simulation::GetActive();
  auto* grid = sim->GetGrid();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();

  // commit all changes
  const auto& all_exec_ctxts = sim->GetAllExecCtxts();
  all_exec_ctxts[0]->TearDownIterationAll(all_exec_ctxts);

  if (!is_gpu_environment_initialized_ && param->use_gpu_) {
    InitializeGPUEnvironment();
    is_gpu_environment_initialized_ = true;
  }

  if (param->bound_space_) {
    rm->ApplyOnAllElementsParallel(*bound_space_);
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

std::vector<Operation> Scheduler::GetScheduleOps() {
  std::vector<Operation> scheduled_ops;
  scheduled_ops.reserve(operations_.size());
  auto* param = Simulation::GetActive()->GetParam();
  for (auto& op : operations_) {
    // special condition for displacement
    if (op.name_ == "displacement" &&
        (!param->run_mechanical_interactions_ || !displacement_->UseCpu())) {
      continue;
    }
    if (total_steps_ % op.frequency_ == 0) {
      scheduled_ops.push_back(op);
    }
  }
  return scheduled_ops;
}

}  // namespace bdm
