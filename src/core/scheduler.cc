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
  std::vector<std::string> default_ops = {"update run displacement",
                                          "bound space",
                                          "biology module",
                                          "displacement",
                                          "discretization",
                                          "distribute run displacement info",
                                          "diffusion"};

  // Schedule the default operations
  for (auto& def_op : default_ops) {
    ScheduleOp(NewOperation(def_op));
  }

  visualize_op_ = NewOperation("visualize");
  setup_iteration_op_ = NewOperation("set up iteration");
  sort_balance_op_ = NewOperation("load balancing");
  teardown_iteration_op_ = NewOperation("tear down iteration");
  update_environment_op_ = NewOperation("update environment");

  outstanding_operations_ = {"load balancing", "visualize"};

  protected_ops_ = {"update run displacement", "biology module",
                    "discretization", "distribute run displacment info"};
}

Scheduler::~Scheduler() {
  for (auto* op : all_ops_) {
    delete op;
  }
  delete visualize_op_;
  delete sort_balance_op_;
  delete update_environment_op_;
  delete setup_iteration_op_;
  delete teardown_iteration_op_;
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

void Scheduler::ScheduleOp(Operation* op) {
  all_ops_.push_back(op);
  ops_to_add_.push_back(op);
}

void Scheduler::UnscheduleOp(Operation* op) {
  // We must not unschedule a protected operation
  if (std::find(protected_ops_.begin(), protected_ops_.end(), op->name_) !=
      protected_ops_.end()) {
    Log::Warning("Scheduler::UnscheduleOp",
                 "You tried to unschedule the protected operation ", op->name_,
                 "! This request was ignored.");
    return;
  }

  // Check if the requested operation is even scheduled
  bool not_in_scheduled_ops = true;
  ForAllScheduledOperations([&](Operation* scheduled_op) {
    if (op == scheduled_op) {
      not_in_scheduled_ops = false;
    }
  });

  if (not_in_scheduled_ops) {
    Log::Warning("Scheduler::UnscheduleOp",
                 "You tried to unschedule the non-scheduled operation ",
                 op->name_, "! This request was ignored.");
    return;
  }

  // 'Unschedule' outstanding operations
  if (std::find(outstanding_operations_.begin(), outstanding_operations_.end(),
                op->name_) != outstanding_operations_.end()) {
    op->standalone_enabled_ = false;
  }

  ops_to_remove_.push_back(op);
}

std::vector<std::string> Scheduler::GetListOfScheduledSimObjectOps() const {
  std::vector<std::string> list;
  for (auto* op : scheduled_sim_object_ops_) {
    list.push_back(op->name_);
  }
  return list;
}

std::vector<std::string> Scheduler::GetListOfScheduledStandaloneOps() const {
  std::vector<std::string> list;
  for (auto* op : scheduled_standalone_ops_) {
    list.push_back(op->name_);
  }
  return list;
}

std::vector<Operation*> Scheduler::GetOps(const std::string& name) {
  std::vector<Operation*> ret;
  if (std::find(protected_ops_.begin(), protected_ops_.end(), name) !=
      protected_ops_.end()) {
    Log::Warning("Scheduler::GetOps", "The operation '", name,
                 "' is a protected operation. Request ignored.");
    return ret;
  }

  for (auto it = all_ops_.begin(); it != all_ops_.end(); ++it) {
    if ((*it)->name_ == name) {
      ret.push_back(*it);
    }
  }

  return ret;
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
  ForAllScheduledOperations([&](Operation* op) {
    if (total_steps_ % op->frequency_ == 0) {
      op->SetUp();
    }
  });
}

void Scheduler::TearDownOps() {
  ForAllScheduledOperations([&](Operation* op) {
    if (total_steps_ % op->frequency_ == 0) {
      op->TearDown();
    }
  });
}

void Scheduler::RunScheduledOps() {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();
  auto batch_size = param->scheduling_batch_size_;

  // Run the sim object operations
  std::vector<Operation*> sim_object_ops;
  for (auto* op : scheduled_sim_object_ops_) {
    if (total_steps_ % op->frequency_ == 0) {
      sim_object_ops.push_back(op);
    }
  }
  RunAllScheduledOps functor(sim_object_ops);

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
  Timing::Time(setup_iteration_op_->name_, [&]() { (*setup_iteration_op_)(); });

  // We cannot put sort and balance in the list of scheduled_standalone_ops_,
  // because numa-aware data structures would be invalidated:
  // ```
  //  SetUpOps() <-- (1)
  //  RunScheduledOps() <-- rebalance numa domains
  //  TearDownOps() <-- indexing with SoHandles are different than at (1)
  // ```
  if (total_steps_ % sort_balance_op_->frequency_ == 0) {
    Timing::Time(sort_balance_op_->name_, [&]() { (*sort_balance_op_)(); });
  }

  // We need to update the environment BEFORE setting up the operations, as
  // some operations depend on the grid members to be up to date (e.g. GPU
  // displacement operation)
  Timing::Time(update_environment_op_->name_,
               [&]() { (*update_environment_op_)(); });

  SetUpOps();
  RunScheduledOps();
  TearDownOps();

  Timing::Time(teardown_iteration_op_->name_,
               [&]() { (*teardown_iteration_op_)(); });

  if (total_steps_ % visualize_op_->frequency_ == 0) {
    Timing::Time(visualize_op_->name_, [&]() { (*visualize_op_)(); });
  }
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
