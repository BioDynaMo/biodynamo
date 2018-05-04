#include "simulation_backup.h"

namespace bdm {

SimulationBackup::SimulationBackup(const std::string& backup_file,
                                   const std::string& restore_file)
    : backup_file_(backup_file), restore_file_(restore_file) {
  backup_ = true;
  if (backup_file_ == "") {
    backup_ = false;
    Log::Warning("SimulationBackup",
                 "No backup file name given. No backups will be made!");
  } else if (backup_file_ == restore_file_) {
    Log::Warning(
        "SimulationBackup",
        "Backup file is identical to restore file. Will be overriden after "
        "restore.");
  }

  if (restore_file_ == "") {
    restore_ = false;
  } else if (!FileExists(restore_file_)) {
    Log::Fatal("SimulationBackup", "Given restore file does not exist.");
  }
}

size_t SimulationBackup::GetSimulationStepsFromBackup() {
  if (restore_) {
    IntegralTypeWrapper<size_t>* wrapper = nullptr;
    bdm::GetPersistentObject(restore_file_.c_str(), kSimulationStepName.c_str(),
                             wrapper);
    return wrapper->Get();
  } else {
    Log::Fatal("SimulationBackup",
               "Requested to restore data, but no restore file given.");
    return 0;
  }
}

bool SimulationBackup::BackupEnabled() { return backup_; }

bool SimulationBackup::RestoreEnabled() { return restore_; }

const std::string SimulationBackup::kResouceManagerName = "resource_manager";
const std::string SimulationBackup::kSimulationStepName =
    "completed_simulation_steps";
const std::string SimulationBackup::kRuntimeVariableName = "runtime_variable";

std::vector<std::function<void()>> SimulationBackup::after_restore_event_ = {};

}  // namespace bdm
