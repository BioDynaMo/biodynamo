// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/simulation_backup.h"

namespace bdm {

SimulationBackup::SimulationBackup(const std::string& backup_file,
                                   const std::string& restore_file)
    : backup_file(backup_file), restore_file(restore_file) {
  backup_ = true;
  if (backup_file == "") {
    backup_ = false;
    Log::Info("SimulationBackup",
              "No backup file name given. No backups will be made!");
  } else if (backup_file == restore_file) {
    Log::Warning(
        "SimulationBackup",
        "Backup file is identical to restore file. Will be overriden after "
        "restore.");
  }

  if (restore_file == "") {
    restore_ = false;
  } else if (!FileExists(restore_file)) {
    Log::Fatal("SimulationBackup",
               "Given restore file does not exist: ", restore_file);
  }
}

size_t SimulationBackup::GetSimulationStepsFromBackup() {
  if (restore_) {
    IntegralTypeWrapper<size_t>* wrapper = nullptr;
    bdm::GetPersistentObject(restore_file.c_str(), kSimulationStepName.c_str(),
                             wrapper);
    if (wrapper != nullptr) {
      return wrapper->Get();
    } else {
      Log::Fatal("SimulationBackup", "Failed to retrieve SimulationSteps.");
      return 0;
    }
  } else {
    Log::Fatal("SimulationBackup",
               "Requested to restore data, but no restore file given.");
    return 0;
  }
}

bool SimulationBackup::BackupEnabled() { return backup_; }

bool SimulationBackup::RestoreEnabled() { return restore_; }

const std::string SimulationBackup::kSimulationName = "simulation";
const std::string SimulationBackup::kSimulationStepName =
    "completed_simulation_steps";
const std::string SimulationBackup::kRuntimeVariableName = "runtime_variable";

std::vector<std::function<void()>> SimulationBackup::after_restore_event_ = {};

}  // namespace bdm
