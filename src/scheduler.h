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
// #include "commit_op.h"
#include "diffusion_op.h"
#include "displacement_op_cpu.h"
// #include "gpu/gpu_helper.h"
#include "op_timer.h"
#include "resource_manager.h"
// #include "simulation_backup.h"

#include "log.h"
#include "param.h"
#include "simulation.h"
// #include "visualization/catalyst_adaptor.h"

namespace bdm {

class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler();

  virtual ~Scheduler();

  void Simulate(uint64_t steps);

  /// This function returns the numer of simulated steps (=iterations).
  uint64_t GetSimulatedSteps() const;

 protected:
  uint64_t total_steps_ = 0;

  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute(bool last_iteration);

 private:
  // SimulationBackup* backup_ = nullptr;
  uint64_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  // CatalystAdaptor<>* visualization_ = nullptr;  //!
  bool is_gpu_environment_initialized_ = false;

  // CommitOp commit_ = CommitOp>("commit");
  DiffusionOp diffusion_;
  BiologyModuleOp biology_;
  DisplacementOpCpu physics_;
  BoundSpace bound_space_;

  /// Backup the simulation. Backup interval based on `Param::backup_interval_`
  void Backup();

  /// Restore the simulation if requested at the right time
  /// @param steps number of simulation steps for a `Simulate` call
  /// @return if `Simulate` should return early
  bool Restore(uint64_t* steps);

  void CommitChangesAndUpdateReferences();

  // TODO(lukas, ahmad) After https://trello.com/c/0D6sHCK4 has been resolved
  // think about a better solution, because some operations are executed twice
  // if Simulate is called with one timestep.
  void Initialize();
};

}  // namespace bdm

#endif  // SCHEDULER_H_
