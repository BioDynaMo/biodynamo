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
#include "core/operation/visualization_op.h"
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
  std::vector<std::string> default_op_names = {
      "update run displacement",
      "bound space",
      "biology module",
      "displacement",
      "discretization",
      "distribute run displacement info",
      "diffusion"};

  std::vector<std::string> pre_scheduled_ops_names = {"set up iteration",
                                                      "update environment"};
  // We cannot put sort and balance in the list of scheduled_standalone_ops_,
  // because numa-aware data structures would be invalidated:
  // ```
  //  SetUpOps() <-- (1)
  //  RunScheduledOps() <-- rebalance numa domains
  //  TearDownOps() <-- indexing with SoHandles is different than at (1)
  // ```
  // Also, must be done before TearDownIteration, because that introduces new
  // sim objects that are not yet in the environment (which load balancing
  // relies on)
  std::vector<std::string> post_scheduled_ops_names = {
      "load balancing", "tear down iteration", "visualize"};

  // Schedule the default operations
  for (auto& def_op : default_op_names) {
    ScheduleOp(NewOperation(def_op), OpType::kSchedule);
  }

  for (auto& def_op : pre_scheduled_ops_names) {
    ScheduleOp(NewOperation(def_op), OpType::kPreSchedule);
  }

  for (auto& def_op : post_scheduled_ops_names) {
    ScheduleOp(NewOperation(def_op), OpType::kPostSchedule);
  }

  protected_op_names_ = {
      "update run displacement", "biology module",
      "discretization",          "distribute run displacment info",
      "set up iteration",        "update environment",
      "tear down iteration"};

  GetOps("visualize")[0]->GetImplementation<VisualizationOp>()->Initialize();
  // ScheduleOps();
}

Scheduler::~Scheduler() {
  for (auto* op : all_ops_) {
    delete op;
  }
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

void Scheduler::ScheduleOp(Operation* op, OpType op_type) {
  // Check if operation is already in all_ops_ (could be the case when
  // trying to reschedule a previously unscheduled operation)
  if (std::find(all_ops_.begin(), all_ops_.end(), op) == all_ops_.end()) {
    all_ops_.push_back(op);
  }

  schedule_ops_.push_back(std::make_pair(op_type, op));
}

void Scheduler::UnscheduleOp(Operation* op) {
  // We must not unschedule a protected operation
  if (std::find(protected_op_names_.begin(), protected_op_names_.end(),
                op->name_) != protected_op_names_.end()) {
    Log::Warning("Scheduler::UnscheduleOp",
                 "You tried to unschedule the protected operation ", op->name_,
                 "! This request was ignored.");
    return;
  }

  // Check if the requested operation is even scheduled
  bool not_in_scheduled_ops = true;
  ForAllOperations([&](Operation* scheduled_op) {
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

  unschedule_ops_.push_back(op);
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

  // Check if a protected op is trying to be fetched
  if (std::find(protected_op_names_.begin(), protected_op_names_.end(), name) !=
      protected_op_names_.end()) {
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

void Scheduler::RunPreScheduledOps() {
  for (auto* pre_op : pre_scheduled_ops_) {
    if (total_steps_ % pre_op->frequency_ == 0) {
      Timing::Time(pre_op->name_, [&]() { (*pre_op)(); });
    }
  }
}

void Scheduler::RunScheduledOps() {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();
  auto batch_size = param->scheduling_batch_size_;

  SetUpOps();

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

  TearDownOps();
}

void Scheduler::RunPostScheduledOps() {
  for (auto* post_op : post_scheduled_ops_) {
    if (total_steps_ % post_op->frequency_ == 0) {
      Timing::Time(post_op->name_, [&]() { (*post_op)(); });
    }
  }
}

void Scheduler::Execute() {
  ScheduleOps();

  RunPreScheduledOps();
  RunScheduledOps();
  RunPostScheduledOps();
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
  for (auto it = schedule_ops_.begin(); it != schedule_ops_.end();) {
    auto op_type = it->first;
    auto* op = it->second;

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

    // Check operation type and append to corresponding list
    switch (op_type) {
      case kPreSchedule:
        pre_scheduled_ops_.push_back(op);
        break;
      case kPostSchedule:
        post_scheduled_ops_.push_back(op);
        break;
      default:
        if (op->IsStandalone()) {
          scheduled_standalone_ops_.push_back(op);
        } else {
          scheduled_sim_object_ops_.push_back(op);
        }
    }

    // Remove operation from schedule_ops_
    it = schedule_ops_.erase(it);
  }

  // Unschedule requested operations
  for (auto it = unschedule_ops_.begin(); it != unschedule_ops_.end();) {
    auto* op = *it;

    // Lists of operations that should be considered for unscheduling
    std::vector<std::vector<Operation*>*> op_lists = {
        &scheduled_sim_object_ops_, &scheduled_standalone_ops_,
        &pre_scheduled_ops_, &post_scheduled_ops_};

    for (auto* op_list : op_lists) {
      for (auto it2 = op_list->begin(); it2 != op_list->end(); ++it2) {
        if (op == (*it2)) {
          it2 = op_list->erase(it2);
          goto label;
        }
      }
    }
  label:
    it = unschedule_ops_.erase(it);
  }
}

}  // namespace bdm
