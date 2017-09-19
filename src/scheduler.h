#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <chrono>
#include <string>

#include "biology_module_op.h"
// TODO(lukas) remove once backup and visualization are multicell enabled
#include "cell.h"
#include "displacement_op.h"
#include "op_timer.h"
#include "resource_manager.h"
#include "simulation_backup.h"

#include "param.h"
#include "visualization/catalyst_adaptor.h"

namespace bdm {

template <typename TResourceManager = ResourceManager<>,
          typename TGrid = Grid<>>
class Scheduler {
 public:
  using Clock = std::chrono::high_resolution_clock;

  Scheduler()
      : backup_(SimulationBackup("", "")), grid_(&TGrid::GetInstance()) {
    if (Param::use_paraview_) {
      visualization_ = CatalystAdaptor::GetInstance();
      visualization_->Initialize("../src/visualization/simple_pipeline.py");
    }
  }

  Scheduler(const std::string& backup_file, const std::string& restore_file)
      : backup_(SimulationBackup(backup_file, restore_file)),
        grid_(&TGrid::GetInstance()) {
    if (backup_.RestoreEnabled()) {
      restore_point_ = backup_.GetSimulationStepsFromBackup();
    }
    if (Param::use_paraview_) {
      visualization_ = CatalystAdaptor::GetInstance();
      visualization_->Initialize("../src/visualization/simple_pipeline.py");
    }
  }

  virtual ~Scheduler() {
    if (Param::use_paraview_) {
      visualization_->Finalize();
    }
  }

  void Simulate(unsigned steps) {
    // TODO(lukas) backup and restore should work for every simulation object in
    // ResourceManager
    if (backup_.RestoreEnabled() && restore_point_ > total_steps_ + steps) {
      total_steps_ += steps;
      return;
    } else if (backup_.RestoreEnabled() && restore_point_ > total_steps_ &&
               restore_point_ < total_steps_ + steps) {
      // Restore
      backup_.Restore();

      steps = total_steps_ + steps - restore_point_;
      total_steps_ = restore_point_;
    }

    grid_->Initialize();

    for (unsigned step = 0; step < steps; step++) {
      // Simulate
      Execute();

      // Visualize
      auto rm = TResourceManager::Get();
      auto cells = rm->template Get<Cell>();
      if (Param::use_paraview_) {
        double time = Param::kSimulationTimeStep * total_steps_;
        visualization_->CoProcess(cells, time, total_steps_, step == steps - 1);
      }

      total_steps_++;

      // Backup
      using std::chrono::seconds;
      using std::chrono::duration_cast;
      if (backup_.BackupEnabled() &&
          duration_cast<seconds>(Clock::now() - last_backup_).count() >=
              Param::backup_every_x_seconds_) {
        last_backup_ = Clock::now();
        backup_.Backup(total_steps_);
      }
    }
  }

 protected:
  /// Executes one step.
  /// This design makes testing more convenient
  virtual void Execute() {
    auto rm = TResourceManager::Get();
    static const auto commit = [](auto* sim_objects, uint16_t type_idx) {
      sim_objects->Commit();
    };

    {
      Timing timing("neighbors");
      grid_->UpdateGrid();
    }
    rm->ApplyOnAllTypes(biology_);
    for (int i = 0; i < 10; i++)
      rm->ApplyOnAllTypes(physics_);
    rm->ApplyOnAllTypes(commit);
  }

 private:
  SimulationBackup backup_;
  size_t total_steps_ = 0;
  size_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  CatalystAdaptor* visualization_ = nullptr;

  OpTimer<BiologyModuleOp> biology_ = OpTimer<BiologyModuleOp>("biology");
  OpTimer<DisplacementOp<>> physics_ = OpTimer<DisplacementOp<>>("physics");

  TGrid* grid_;
};

}  // namespace bdm

#endif  // SCHEDULER_H_
