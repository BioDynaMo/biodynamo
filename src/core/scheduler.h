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

#include <chrono>
#include <functional>
#include <set>
#include <string>
#include <vector>
#include "core/operation/operation.h"
#include "core/param/param.h"

namespace bdm {

class SimObject;
class SimulationBackup;
class VisualizationAdaptor;
class RootAdaptor;
class BoundSpace;
class DisplacementOp;
class DiffusionOp;

class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler();

  virtual ~Scheduler();

  void Simulate(uint64_t steps);

  /// This function returns the numer of simulated steps (=iterations).
  uint64_t GetSimulatedSteps() const;

  void AddOperation(Operation* operation);

  /// Remove an operation. However, some operations are protected and cannot
  /// be removed. \see protected_operations_
  /// A request to remove a proteced operation is ignored.
  void RemoveOperation(const std::string& op_name);

  /// Returns a reference to an operation. However, some operations are
  /// protected and will not be returned. \see protected_operations_
  /// If the operation does not exist or is protected, a nullptr will be
  /// returned.
  Operation* GetOperation(const std::string& op_name);

  RootAdaptor* GetRootVisualization() { return root_visualization_; }

 protected:
  uint64_t total_steps_ = 0;

  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute();

 private:
  friend void RunSimObjectsTest(Param::MappedDataArrayMode, uint64_t, bool, bool); 
  SimulationBackup* backup_ = nullptr;
  uint64_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  VisualizationAdaptor* visualization_ = nullptr;  //!
  RootAdaptor* root_visualization_ = nullptr;      //!

  BoundSpace* bound_space_;
  DisplacementOp* displacement_;
  DiffusionOp* diffusion_;

  std::vector<Operation*> operations_;  //!
  std::set<std::string> protected_operations_;

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

  // Decide which operations should be executed
  std::vector<Operation*> GetScheduleOps();
};

}  // namespace bdm

#endif  // CORE_SCHEDULER_H_
