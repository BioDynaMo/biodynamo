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

#include <chrono>
#include <string>

#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/diffusion_op.h"
#include "core/operation/displacement_op.h"
#include "core/operation/op_timer.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/simulation_backup.h"
#include "core/util/log.h"
#include "core/visualization/root/adaptor.h"

namespace bdm {

Scheduler::Scheduler() {
  auto* param = Simulation::GetActive()->GetParam();
  backup_ = new SimulationBackup(param->backup_file_, param->restore_file_);
  if (backup_->RestoreEnabled()) {
    restore_point_ = backup_->GetSimulationStepsFromBackup();
  }
  root_visualization_ = new RootAdaptor();

  // Operations are scheduled in the following order (sub categorated by their
  // operation implementation type, so that actual order may vary)
  default_ops_ = {"set up exec context",
                  "visualize",
                  "update environment",
                  "first op",
                  "bound space",
                  "biology module",
                  "displacement",
                  "discretization",
                  "diffusion",
                  "last op",
                  "tear down exec context"};

  // Schedule the default operations
  for (auto& def_op : default_ops_) {
    ScheduleOp(NewOperation(def_op));
  }

  protected_ops_ = {"set up exec context", "tear down exec context", "first op",
                    "biology module",      "discretization",         "last op"};
}

Scheduler::~Scheduler() {
  for (auto* op : unscheduled_ops_) {
    delete op;
  }
  for (auto* op : scheduled_sim_object_ops_) {
    delete op;
  }
  for (auto* op : scheduled_standalone_ops_) {
    delete op;
  }
  // Normally this list of ops is empty, but it can happen that an uninitialized
  // Scheduler gets destructed, leaving all the ops in this vector
  // In such cases we loop over this vector
  for (auto* op : ops_to_add_) {
    delete op;
  }
  delete backup_;
  delete root_visualization_;
}

void Scheduler::Simulate(uint64_t steps) {
  if (Restore(&steps)) {
    return;
  }

  Initialize();
  for (unsigned step = 0; step < steps; step++) {
    Execute();

    total_steps_++;
    Backup();
  }
}

uint64_t Scheduler::GetSimulatedSteps() const { return total_steps_; }

TimingAggregator* Scheduler::GetOpTimes() { return &op_times_; }

void Scheduler::ScheduleOp(Operation* op) { ops_to_add_.push_back(op); }

void Scheduler::UnscheduleOp(Operation* op) {
  if (std::find(protected_ops_.begin(), protected_ops_.end(), op->name_) !=
      protected_ops_.end()) {
    Log::Warning("Scheduler::UnscheduleOp",
                 "You tried to remove the protected operation ", op->name_,
                 "! This request was ignored.");
    return;
  }

  ops_to_remove_.push_back(op);
}

Operation* Scheduler::GetDefaultOp(const std::string& name) {
  // Check if Initialize has been called, otherwise the scheduler operation
  // lists will be empty
  if (!initialized_) {
    Log::Fatal("Scheduler::GetDefaultOp",
               "The scheduler has not been yet initialized!");
    return nullptr;
  }

  if (std::find(default_ops_.begin(), default_ops_.end(), name) ==
      default_ops_.end()) {
    Log::Warning("Scheduler::GetDefaultOp", "The operation '", name,
                 "' is not a default operation. Request ignored.");
    return nullptr;
  }

  for (auto it = scheduled_sim_object_ops_.begin();
       it != scheduled_sim_object_ops_.end(); ++it) {
    if ((*it)->name_ == name) {
      return *it;
    }
  }

  for (auto it = scheduled_standalone_ops_.begin();
       it != scheduled_standalone_ops_.end(); ++it) {
    if ((*it)->name_ == name) {
      return *it;
    }
  }

  return nullptr;
}

struct RunAllScheduledOps : Functor<void, SimObject*, SoHandle> {
  RunAllScheduledOps(std::vector<Operation*>& scheduled_ops)
      : scheduled_ops_(scheduled_ops) {
    sim_ = Simulation::GetActive();
  }

  void operator()(SimObject* so, SoHandle) override {
    sim_->GetExecutionContext()->Execute(so, scheduled_ops_);
  }

  Simulation* sim_;
  std::vector<Operation*>& scheduled_ops_;
};

void Scheduler::SetUpOps() {
  for (auto* op : scheduled_sim_object_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      op->SetUp();
    }
  }

  for (auto* op : scheduled_standalone_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      op->SetUp();
    }
  }
}

void Scheduler::TearDownOps() {
  for (auto* op : scheduled_sim_object_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      op->TearDown();
    }
  }

  for (auto* op : scheduled_standalone_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      op->TearDown();
    }
  }
}

void Scheduler::RunScheduledOps() {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();
  auto batch_size = param->scheduling_batch_size_;

  // Run the row-wise operations
  std::vector<Operation*> row_wise_operations;
  for (auto* op : scheduled_sim_object_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      row_wise_operations.push_back(op);
    }
  }
  RunAllScheduledOps functor(row_wise_operations);

  Timing::Time("sim object ops", [&]() {
    rm->ApplyOnAllElementsParallelDynamic(batch_size, functor);
  });

  // Run the column-wise operations
  for (auto* op : scheduled_standalone_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      Timing::Time(op->name_, [&]() { (*op)(); });
    }
  }
}

void Scheduler::Execute() {
  ScheduleOps();

  SetUpOps();

  RunScheduledOps();

  TearDownOps();
}

void Scheduler::Backup() {
  using std::chrono::duration_cast;
  using std::chrono::seconds;
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
  auto* env = sim->GetEnvironment();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();

  // commit all changes
  const auto& all_exec_ctxts = sim->GetAllExecCtxts();
  all_exec_ctxts[0]->TearDownIterationAll(all_exec_ctxts);

  if (param->bound_space_) {
    auto* bound_space = NewOperation("bound space");
    rm->ApplyOnAllElementsParallel(*bound_space);
    delete bound_space;
  }
  env->Update();
  int lbound = env->GetDimensionThresholds()[0];
  int rbound = env->GetDimensionThresholds()[1];
  rm->ApplyOnAllDiffusionGrids([&](DiffusionGrid* dgrid) {
    // Create data structures, whose size depend on the env dimensions
    dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
    // Initialize data structures with user-defined values
    dgrid->RunInitializers();
  });

  ScheduleOps();

  initialized_ = true;
}

// Schedule the operations
void Scheduler::ScheduleOps() {
  auto* param = Simulation::GetActive()->GetParam();
  // Add requested operations
  for (auto it = ops_to_add_.begin(); it != ops_to_add_.end();) {
    auto* op = *it;
    // Enable GPU operation implementations (if available) if CUDA or OpenCL
    // flags are set
    if (param->compute_target_ == "cuda" &&
        op->IsComputeTargetSupported(kCuda)) {
      op->SelectComputeTarget(kCuda);
    } else if (param->compute_target_ == "opencl" &&
               op->IsComputeTargetSupported(kOpenCl)) {
      op->SelectComputeTarget(kOpenCl);
    } else {
      op->SelectComputeTarget(kCpu);
    }

    if (op->IsStandalone()) {
      scheduled_standalone_ops_.push_back(op);
    } else {
      scheduled_sim_object_ops_.push_back(op);
    }

    // Remove operation from ops_to_add_
    it = ops_to_add_.erase(it);
  }

  // Remove requested operations
  for (auto it = ops_to_remove_.begin(); it != ops_to_remove_.end();) {
    auto* op = *it;
    // Check scheduled row-wise operations list
    for (auto it2 = scheduled_sim_object_ops_.begin();
         it2 != scheduled_sim_object_ops_.end(); ++it2) {
      if (op == (*it2)) {
        // Add to list of unscheduled operations
        unscheduled_ops_.push_back(op);
        it2 = scheduled_sim_object_ops_.erase(it2);
        goto label;
      }
    }

    // Check scheduled column-wise operations list
    for (auto it2 = scheduled_standalone_ops_.begin();
         it2 != scheduled_standalone_ops_.end(); ++it2) {
      if (op == (*it2)) {
        // Add to list of unscheduled operations
        unscheduled_ops_.push_back(op);
        it2 = scheduled_standalone_ops_.erase(it2);
        goto label;
      }
    }
  label:
    it = ops_to_remove_.erase(it);
  }
}

}  // namespace bdm
