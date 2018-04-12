#ifndef SIMULATION_BACKUP_H_
#define SIMULATION_BACKUP_H_

#include <sstream>
#include <string>
#include "resource_manager.h"

#include "io_util.h"
#include "log.h"

namespace bdm {

/// SimulationBackup is responsible for backing up and restoring all relevant
/// simulation information.
class SimulationBackup {
 public:
  // object names for root file
  static const std::string kResouceManagerName;
  static const std::string kSimulationStepName;
  static const std::string kRuntimeVariableName;

  SimulationBackup(const std::string& backup_file,
                   const std::string& restore_file);

  template <typename TResourceManager = ResourceManager<>>
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
      f.Get()->WriteObject(TResourceManager::Get(),
                           kResouceManagerName.c_str());
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

  template <typename TResourceManager = ResourceManager<>>
  void Restore() {
    if (!restore_) {
      Log::Fatal("SimulationBackup",
                 "Requested to restore data, but no restore file given.");
    }

    TFileRaii file(TFile::Open(restore_file_.c_str()));
    RuntimeVariables* restored_rv;
    file.Get()->GetObject(kRuntimeVariableName.c_str(), restored_rv);
    // check if runtime variables are the same
    if (!(RuntimeVariables() == *restored_rv)) {
      Log::Warning("SimulationBackup",
                   "Restoring simulation executed on a different system!");
    }
    TResourceManager* restored_rm = nullptr;
    file.Get()->GetObject(kResouceManagerName.c_str(), restored_rm);
    TResourceManager::instance_ =
        std::unique_ptr<TResourceManager>(restored_rm);
    // TODO(lukas) random number generator, statics (e.g. Param)
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

#endif  // SIMULATION_BACKUP_H_
