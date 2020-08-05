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
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/simulation_backup.h"
#include "core/util/log.h"
#include "core/visualization/root/adaptor.h"
#include "core/visualization/visualization_adaptor.h"

namespace bdm {

struct FirstOp : OperationImpl {
  void operator()(SimObject* so) override { so->UpdateRunDisplacement(); }

 private:
  static bool registered_;
};

REGISTER_OP(FirstOp, "first op", kCpu);

struct LastOp : OperationImpl {
  void operator()(SimObject* so) override {
    so->ApplyRunDisplacementForAllNextTs();
  }

 private:
  static bool registered_;
};

REGISTER_OP(LastOp, "last op", kCpu);

struct BiologyModuleOp : OperationImpl {
  void operator()(SimObject* so) override { so->RunBiologyModules(); }

 private:
  static bool registered_;
};

REGISTER_OP(BiologyModuleOp, "biology module", kCpu);

struct DiscretizationOp : OperationImpl {
  void operator()(SimObject* so) override { so->RunDiscretization(); }

 private:
  static bool registered_;
};

REGISTER_OP(DiscretizationOp, "discretization", kCpu);

Scheduler::Scheduler() {
  auto* param = Simulation::GetActive()->GetParam();
  backup_ = new SimulationBackup(param->backup_file_, param->restore_file_);
  if (backup_->RestoreEnabled()) {
    restore_point_ = backup_->GetSimulationStepsFromBackup();
  }
  visualization_ = VisualizationAdaptor::Create(param->visualization_engine_);
  root_visualization_ = new RootAdaptor();

  ops_to_add_ = {GET_OP("first op"),       GET_OP("bound space"),
                 GET_OP("biology module"), GET_OP("displacement"),
                 GET_OP("discretization"), GET_OP("diffusion"),
                 GET_OP("last op")};
  protected_operations_ = {"first op", "biology module", "discretization",
                           "last op"};
}

Scheduler::~Scheduler() {
  delete backup_;
  delete root_visualization_;
  if (visualization_) {
    delete visualization_;
  }
  auto* param = Simulation::GetActive()->GetParam();
  if (param->statistics_) {
    std::cout << gStatistics << std::endl;
  }
  // We do not delete operations here; we leave that to OperationRegistry
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

void Scheduler::AddOperation(Operation* op) { ops_to_add_.push_back(op); }

void Scheduler::RemoveOperation(const std::string& name) {
  if (protected_operations_.find(name) != protected_operations_.end()) {
    Log::Warning("Scheduler::GetOperation",
                 "You tried to remove the protected operation ", name,
                 "! This request was ignored.");
    return;
  }

  ops_to_remove_.push_back(GET_OP(name));
}

struct RunAllScheduldedOps : Functor<void, SimObject*, SoHandle> {
  RunAllScheduldedOps(std::vector<Operation*>& scheduled_ops)
      : scheduled_ops_(scheduled_ops) {
    sim_ = Simulation::GetActive();
  }

  void operator()(SimObject* so, SoHandle) override {
    sim_->GetExecutionContext()->Execute(so, scheduled_ops_);
  }

  Simulation* sim_;
  std::vector<Operation*>& scheduled_ops_;
};

void Scheduler::RunScheduledOps() {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* env = sim->GetEnvironment();
  auto* param = sim->GetParam();
  auto batch_size = param->scheduling_batch_size_;

  // Run the row-wise operations
  std::vector<Operation*> row_wise_operations;
  for (auto* op : scheduled_row_wise_operations_) {
    if (total_steps_ % op->frequency_ == 0) {
      row_wise_operations.push_back(op);
    }
  }
  RunAllScheduldedOps functor(row_wise_operations);
  rm->ApplyOnAllElementsParallelDynamic(batch_size, functor);

  // Run the column-wise operations
  for (auto* op : scheduled_column_wise_operations_) {
    if (total_steps_ % op->frequency_ == 0) {
      (*op)();
    }
  }
}

void Scheduler::Execute() {
  auto* sim = Simulation::GetActive();
  auto* env = sim->GetEnvironment();

  Timing::Time("Set up exec context", [&]() {
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
  });

  Timing::Time("visualize", [&]() {
    if (visualization_ != nullptr) {
      visualization_->Visualize();
    }
  });
  Timing::Time("neighbors", [&]() { env->Update(); });

  RunScheduledOps();

  // finish updating sim objects
  Timing::Time("Tear down exec context", [&]() {
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->TearDownIterationAll(all_exec_ctxts);
  });
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
    rm->ApplyOnAllElementsParallel(*GET_OP("bound space"));
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
    if (param->compute_target_ == "cuda" &&
        op->IsComputeTargetSupported(kCuda)) {
      op->SelectComputeTarget(kCuda);
    } else if (param->compute_target_ == "opencl" &&
               op->IsComputeTargetSupported(kOpenCl)) {
      op->SelectComputeTarget(kOpenCl);
    } else {
      op->SelectComputeTarget(kCpu);
    }
    if (op->IsRowWise()) {
      scheduled_row_wise_operations_.push_back(op);
    } else {
      scheduled_column_wise_operations_.push_back(op);
    }
    // Remove operation from ops_to_add_
    it = ops_to_add_.erase(it);
  }

  // Remove requested operations
  for (auto* op : ops_to_remove_) {
    // Check scheduled row-wise operations list
    for (auto it = scheduled_row_wise_operations_.begin();
         it != scheduled_row_wise_operations_.end(); ++it) {
      if (op->name_ == (*it)->name_) {
        scheduled_row_wise_operations_.erase(it);
        return;
      }
    }

    // Check scheduled column-wise operations list
    for (auto it = scheduled_column_wise_operations_.begin();
         it != scheduled_column_wise_operations_.end(); ++it) {
      if (op->name_ == (*it)->name_) {
        scheduled_column_wise_operations_.erase(it);
        return;
      }
    }
  }
}

}  // namespace bdm
