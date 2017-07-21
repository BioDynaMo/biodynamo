#ifndef SIMULATION_BACKUP_H_
#define SIMULATION_BACKUP_H_

#include <string>
#include <TError.h>

#include "io_util.h"

namespace bdm {

// TODO document
class SimulationBackup {
public:
  SimulationBackup(const std::string& backup_file, const std::string& restore_file);

  template <typename Container>
  void Backup(Container* cells, size_t completed_simulation_steps) {
    // TODO(lukas)  random number generator; all statics (e.g. Param)

    // create temporary file
    // if application crashes during backup; last backup is not currupted
    std::stringstream tmp_file;
    tmp_file << "tmp_ " << backup_file_;

    // Backup
    TFileRaii f(tmp_file.str(), "UPDATE");
    f.Get()->WriteObject(cells, kCellName.c_str());
    f.Get()->WriteObject(&completed_simulation_steps, kSimulationStepName.c_str());
    RuntimeVariables rv;
    f.Get()->WriteObject(&rv, kRuntimeVariableName.c_str());

    // remove last backup file
    remove(backup_file_.c_str());
    // rename temporary file
    rename(tmp_file.str().c_str(), backup_file_.c_str());
  }

  template <typename Container>
  void Restore(Container* cells) {
    if(restore_) {
      TFileRaii file(TFile::Open(restore_file_.c_str()));
      RuntimeVariables* restored_rv;
      file.Get()->GetObject(kRuntimeVariableName.c_str(), restored_rv);
      // check if runtime variables are the same
      if (!(RuntimeVariables() == *restored_rv)) {
        Warning("SimulationBackup", "Restoring simulation executed on a different system!");
      }
      file.Get()->GetObject(kCellName.c_str(), cells);
      // TODO(lukas) random number generator, statics (e.g. Param)
    } else {
      Fatal("SimulationBackup", "Requested to restore data, but no restore file given.");
    }
  }

  size_t GetSimulationStepsFromBackup();

  bool BackupEnabled();

  bool RestoreEnabled();

private:
  bool backup_ = false;
  bool restore_ = true;
  std::string backup_file_;
  std::string restore_file_;

  // object names for root file
  static const std::string kCellName;
  static const std::string kSimulationStepName;
  static const std::string kRuntimeVariableName;
};

}  // namespace bdm

#endif  // SIMULATION_BACKUP_H_
