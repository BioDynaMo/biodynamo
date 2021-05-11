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

#ifndef CORE_SCHEDULER_H_
#define CORE_SCHEDULER_H_

#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/operation/operation.h"
#include "core/param/param.h"
#include "core/util/timing_aggregator.h"

namespace bdm {

class SchedulerTest;
class Agent;
class SimulationBackup;
class VisualizationAdaptor;
class RootAdaptor;
struct BoundSpace;
class MechanicalForcesOp;
struct DiffusionOp;

enum OpType { kSchedule, kPreSchedule, kPostSchedule };

class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler();

  virtual ~Scheduler();

  /// Simulate `steps` number of iterations.
  void Simulate(uint64_t steps);

  /// Simulate until `exit_condition` evaluates to true. \n
  /// The condition will be tested at the beginning of an iteration.\n
  /// e.g. simulate until there are 1000 agents in the simulation
  ///
  ///     scheduler->Simulate([](){
  ///        auto* rm = Simulation::GetActive()->GetResourceManager();
  ///        return rm->GetNumAgents() >= 1000;
  ///     });
  ///
  /// NB: Automated backups and restores are not yet supported
  /// if the simulation uses this simulate function. TODO(lukas)
  void SimulateUntil(const std::function<bool()>& exit_condition);

  /// Finalize simulation initialization or manual changes between
  /// `Simulate` calls.\n
  /// All `Simulate` calls do this automatically, but sometimes
  /// it is useful to do it manually (e.g.for tutorials).
  void FinalizeInitialization();

  /// This function returns the numer of simulated steps (=iterations).
  uint64_t GetSimulatedSteps() const;

  /// Adds the given operation to the list of to be scheduled
  /// operations.
  /// Scheduler takes over ownership of the object `op`.
  /// NB: Don't pass stack objects to this function.
  void ScheduleOp(Operation* op, OpType op_type = OpType::kSchedule);

  void UnscheduleOp(Operation* op);

  /// Returns a vector of operations with the given name.
  /// If the name is in the list of proected ops, this
  /// function returns an empty vector.
  std::vector<Operation*> GetOps(const std::string& name);

  RootAdaptor* GetRootVisualization() { return root_visualization_; }

  TimingAggregator* GetOpTimes();

 protected:
  uint64_t total_steps_ = 0;

  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute();

 private:
  friend void RunAgentsTest(Param::MappedDataArrayMode, uint64_t, bool, bool);
  friend SchedulerTest;

  SimulationBackup* backup_ = nullptr;
  uint64_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  RootAdaptor* root_visualization_ = nullptr;  //!

  /// List of all operations that have been add either as default
  /// or by a call to Scheduler::ScheduleOp.
  /// Scheduler::UnscheduleOp doesn't remove the operation from this
  /// list.
  std::vector<Operation*> all_ops_;  //!
  /// List of operations that are to be added in the upcoming timestep
  std::vector<std::pair<OpType, Operation*>> schedule_ops_;  //!
  /// List of operations that are to be removed in the upcoming timestep
  std::vector<Operation*> unschedule_ops_;  //!
  /// List of operations will be executed as a stand-alone operation
  std::vector<Operation*> scheduled_standalone_ops_;  //!
  /// List of operations will be executed on all agents
  std::vector<Operation*> scheduled_agent_ops_;  //!
  /// List of operations that cannot be affected by the user
  std::vector<std::string> protected_op_names_;  //!
  // Operations that are run before setting up, running and tearing down
  // scheduled operations
  std::vector<Operation*> pre_scheduled_ops_;
  // Operations that are run after setting up, running and tearing down
  // scheduled operations
  std::vector<Operation*> post_scheduled_ops_;
  /// Tracks operations' execution times
  TimingAggregator op_times_;

  /// Backup the simulation. Backup interval based on `Param::backup_interval`
  void Backup();

  /// Restore the simulation if requested at the right time
  /// @param steps number of simulation steps for a `Simulate` call
  /// @return if `Simulate` should return early
  bool Restore(uint64_t* steps);

  // TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
  // think about a better solution, because some operations are executed twice
  // if Simulate is called with one timestep.
  void Initialize();

  /// Runs a lambda for each operation in the specified list of operations
  template <typename Lambda>
  void ForEachOperationInList(const std::vector<Operation*>& operations,
                              Lambda lambda) {
    for (auto* op : operations) {
      lambda(op);
    }
  }

  /// Runs a lambda for each scheduled operation
  template <typename Lambda>
  void ForEachScheduledOperation(Lambda lambda) {
    ForEachOperationInList(scheduled_agent_ops_, lambda);
    ForEachOperationInList(scheduled_standalone_ops_, lambda);
  }

  /// Runs a lambda for each operation that is executed in the Execute() call
  template <typename Lambda>
  void ForEachOperation(Lambda lambda) {
    ForEachScheduledOperation(lambda);
    ForEachOperationInList(pre_scheduled_ops_, lambda);
    ForEachOperationInList(post_scheduled_ops_, lambda);
  }

  /// Return a list of AgentOperations that are scheduled
  std::vector<std::string> GetListOfScheduledAgentOps() const;

  /// Return a list of StandAloneOperations that are scheduled
  std::vector<std::string> GetListOfScheduledStandaloneOps() const;

  // Run the Operation::SetUp() call for each scheduled operation
  void SetUpOps();

  void TearDownOps();

  // Run the operations in scheduled_*_ops_
  void RunScheduledOps();

  // Run the operations in pre_scheduled_ops_ (executed before RunScheduledOps)
  void RunPreScheduledOps();

  // Run the operations in post_scheduled_ops_ (executed after RunScheduledOps)
  void RunPostScheduledOps();

  void ScheduleOps();
};

}  // namespace bdm

#endif  // CORE_SCHEDULER_H_
