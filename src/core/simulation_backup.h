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

#ifndef CORE_SIMULATION_BACKUP_H_
#define CORE_SIMULATION_BACKUP_H_

#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "core/simulation.h"

#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {

/// SimulationBackup is responsible for backing up and restoring all relevant
/// simulation information.
class SimulationBackup {
 public:
  // object names for root file
  static const std::string kSimulationName;
  static const std::string kSimulationStepName;
  static const std::string kRuntimeVariableName;

  /// If a whole simulation is restored from a ROOT file, the new
  /// ResourceManager is not updated before the end. Consequently, during
  /// restore, a custom streamer cannot access the new RessourceManager.
  /// However, they can delay actions that rely on the new RessourceManager
  /// by registering it to `after_restore_event_` and will be executed
  /// after the restore has been finished and the ResourceManager has been
  /// updated.
  static std::vector<std::function<void()>> after_restore_event_;

  /// If `backup_file` is an empty string no backups will be made
  /// If `restore_file` is an empty string no restore will be made
  SimulationBackup(const std::string& backup_file,
                   const std::string& restore_file);

  void Backup(size_t completed_simulation_steps) {
    if (!backup_) {
      Log::Fatal("SimulationBackup",
                 "Requested to backup data, but no backup file given.");
    }

    // create temporary file
    // if application crashes during backup; last backup is not corrupted
    std::stringstream tmp_file;
    tmp_file << "tmp_" << backup_file_;

    // Backup
    {
      TFileRaii f(tmp_file.str(), "UPDATE");
      auto* simulation = Simulation::GetActive();
      f.Get()->WriteObject(simulation, kSimulationName.c_str());
      IntegralTypeWrapper<size_t> wrapper(completed_simulation_steps);
      f.Get()->WriteObject(&wrapper, kSimulationStepName.c_str());
      RuntimeVariables rv;
      f.Get()->WriteObject(&rv, kRuntimeVariableName.c_str());
      // TODO(lukas)  random number generator; all statics (e.g. Param)
    }

    // remove last backup file
    remove(backup_file_.c_str());
    // rename temporary file
    rename(tmp_file.str().c_str(), backup_file_.c_str());
  }

  void Restore() {
    if (!restore_) {
      Log::Fatal("SimulationBackup",
                 "Requested to restore data, but no restore file given.");
    }
    after_restore_event_.clear();

    TFileRaii file(TFile::Open(restore_file_.c_str()));
    RuntimeVariables* restored_rv;
    file.Get()->GetObject(kRuntimeVariableName.c_str(), restored_rv);
    // check if runtime variables are the same
    if (!(RuntimeVariables() == *restored_rv)) {
      Log::Warning("SimulationBackup",
                   "Restoring simulation executed on a different system!");
    }
    Simulation* restored_simulation = nullptr;
    file.Get()->GetObject(kSimulationName.c_str(), restored_simulation);
    Simulation::GetActive()->Restore(std::move(*restored_simulation));
    Log::Info("Scheduler", "Restored simulation from ", restore_file_);
    delete restored_simulation;

    // call all after restore events
    for (auto&& event : after_restore_event_) {
      event();
    }
    after_restore_event_.clear();
  }

  size_t GetSimulationStepsFromBackup();

  bool BackupEnabled();

  bool RestoreEnabled();

 private:
  bool backup_ = false;
  bool restore_ = true;
  std::string backup_file_;
  std::string restore_file_;
};

}  // namespace bdm

#endif  // CORE_SIMULATION_BACKUP_H_
