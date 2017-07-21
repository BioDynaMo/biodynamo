#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <chrono>

#include "biology_module_op.h"
#include "displacement_op.h"
#include "neighbor_nanoflann_op.h"
#include "op_timer.h"
#include "resource_manager.h"
#include "simulation_backup.h"

namespace bdm {

class Scheduler {
 public:
   using Clock = std::chrono::high_resolution_clock;

  Scheduler() : backup_(SimulationBackup("", "")) {}

  Scheduler(const std::string& backup_file, const std::string& restore_file) : backup_(SimulationBackup(backup_file, restore_file)) {
    if(backup_.RestoreEnabled()) {
      restore_point_ = backup_.GetSimulationStepsFromBackup();
    }
  }

  virtual ~Scheduler() {}

  template <typename TCellContainer>
  void Simulate(unsigned steps) {
    auto cells = ResourceManager<TCellContainer>::Get()->GetCells();
    if (backup_.RestoreEnabled() && restore_point_ > total_steps_ + steps ) {
      total_steps_ += steps;
      return;
    } else if (backup_.RestoreEnabled() && restore_point_ > total_steps_ && restore_point_ < total_steps_ + steps) {
      // restore
      // backup_.Restore(&cells);

      steps = total_steps_ + steps - restore_point_;
      total_steps_ = restore_point_;
    }

    while (steps-- > 0) {
      // execute all operations
      neighbor_.Compute(cells);
      biology_.Compute(cells);
      physics_.Compute(cells);

      // commit new and removed cells
      cells->Commit();

      total_steps_++;

      // Backup
      using std::chrono::seconds;
      using std::chrono::duration_cast;
      if (backup_.BackupEnabled() && duration_cast<seconds>(last_backup_ - Clock::now()).count() > Param::kBackupEveryXSeconds) {
        last_backup_ = Clock::now();
        backup_.Backup(cells, total_steps_);
      }
    }
  }

 private:
   SimulationBackup backup_;
   size_t total_steps_;
   size_t restore_point_;
   std::chrono::time_point<Clock> last_backup_ = Clock::now();

   OpTimer<NeighborNanoflannOp> neighbor_ = OpTimer<NeighborNanoflannOp>("neighbor", NeighborNanoflannOp(700));
   OpTimer<BiologyModuleOp> biology_ = OpTimer<BiologyModuleOp>("biology");
   OpTimer<DisplacementOp> physics_ = OpTimer<DisplacementOp>("physics");
};

}  // namespace bdm

#endif  // SCHEDULER_H_
