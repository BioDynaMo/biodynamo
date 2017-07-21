#include "simulation_backup.h"

namespace bdm {

SimulationBackup::SimulationBackup(const std::string& backup_file, const std::string& restore_file) :
  backup_file_(backup_file), restore_file_(restore_file) {
    backup_ = true;
    if(backup_file_ == "") {
      backup_ = false;
      Warning("", "No backup file name given. No backups will be made!");
    } else if(backup_file_ == restore_file_) {
      Warning("", "Backup file is identical to restore file. Will be overriden");
    }

    if(restore_file_ == "") {
      restore_ = false;
    } else if(!FileExists(restore_file_)) {
      Fatal("", "Given restore file does not exist.");
    }
}

size_t SimulationBackup::GetSimulationStepsFromBackup() {
  if(restore_) {
    size_t simulation_step = 0;
    // bdm::GetPersistentObject(restore_file_.c_str(), kSimulationStepName.c_str(), &simulation_step);
    return simulation_step;
  } else {
    Fatal("SimulationBackup", "Requested to restore data, but no restore file given.");
    return 0;
  }
}

bool SimulationBackup::BackupEnabled() {
  return backup_;
}

bool SimulationBackup::RestoreEnabled() {
  return restore_;
}

const std::string SimulationBackup::kCellName = "cells";
const std::string SimulationBackup::kSimulationStepName = "completed_simulation_steps";
const std::string SimulationBackup::kRuntimeVariableName = "runtime_variable";

}  // namespace bdm
