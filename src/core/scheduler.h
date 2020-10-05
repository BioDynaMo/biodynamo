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

#ifndef CORE_SCHEDULER_H_
#define CORE_SCHEDULER_H_

#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/operation/operation.h"
#include "core/param/param.h"
#include "core/util/timing_aggregator.h"

namespace bdm {

class SimObject;
class SimulationBackup;
class VisualizationAdaptor;
class RootAdaptor;
struct BoundSpace;
struct DisplacementOp;
struct DiffusionOp;

class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler();

  virtual ~Scheduler();

  void Simulate(uint64_t steps);

  /// This function returns the numer of simulated steps (=iterations).
  uint64_t GetSimulatedSteps() const;

  /// Adds the given operation to the list of to be scheduled
  /// operations.
  /// Scheduler takes over ownership of the object `op`.
  /// NB: Don't pass stack objects to this function.
  void ScheduleOp(Operation* op);

  void UnscheduleOp(Operation* op);

  /// Returns a vector of operations with the given name.
  /// If the name is in the list of proected ops, this
  /// function returns an empty vector.
  std::vector<Operation*> GetOps(const std::string& name);

  template <typename Lambda>
  void ForAllScheduledOperations(Lambda lambda) {
    for (auto* op : scheduled_sim_object_ops_) {
      lambda(op);
    }

    for (auto* op : scheduled_standalone_ops_) {
      lambda(op);
    }
  }

  /// Return a list of SimObjectOperations that are scheduled
  std::vector<std::string> GetListOfScheduledSimObjectOps() const;

  /// Return a list of StandAloneOperations that are scheduled
  std::vector<std::string> GetListOfScheduledStandaloneOps() const;

  void SetUpOps();

  void TearDownOps();

  void RunScheduledOps();

  void ScheduleOps();

  RootAdaptor* GetRootVisualization() { return root_visualization_; }

  TimingAggregator* GetOpTimes();

 protected:
  uint64_t total_steps_ = 0;

  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute();

 private:
  friend void RunSimObjectsTest(Param::MappedDataArrayMode, uint64_t, bool,
                                bool);
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
  std::vector<Operation*> ops_to_add_;  //!
  /// List of operations that are to be removed in the upcoming timestep
  std::vector<Operation*> ops_to_remove_;  //!
  /// List of operations that were removed from scheduling, but could be reused
  /// later on in a simulation
  std::vector<Operation*> unscheduled_ops_;  //!
  /// List of operations will be executed as a stand-alone operation
  std::vector<Operation*> scheduled_standalone_ops_;  //!
  /// List of operations will be executed on all simulation objects
  std::vector<Operation*> scheduled_sim_object_ops_;  //!
  /// List of operations that cannot be affected by the user
  std::vector<std::string> protected_ops_;  //!
  /// List of operations that cannot be in a scheduled_*_ops_ list, but could be
  /// unscheduled nevertheless
  std::vector<std::string> outstanding_operations_;  //!
  /// Tracks operations' execution times
  TimingAggregator op_times_;
  Operation* visualize_op_ = nullptr;
  Operation* update_environment_op_ = nullptr;
  Operation* sort_balance_op_ = nullptr;
  Operation* setup_iteration_op_ = nullptr;
  Operation* teardown_iteration_op_ = nullptr;

  /// Backup the simulation. Backup interval based on `Param::backup_interval_`
  void Backup();

  /// Restore the simulation if requested at the right time
  /// @param steps number of simulation steps for a `Simulate` call
  /// @return if `Simulate` should return early
  bool Restore(uint64_t* steps);

  // TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
  // think about a better solution, because some operations are executed twice
  // if Simulate is called with one timestep.
  void Initialize();
};

}  // namespace bdm

#endif  // CORE_SCHEDULER_H_
