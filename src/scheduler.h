#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <chrono>
#include <string>

#include "biology_module_op.h"
// TODO(lukas) remove once backup and visualization are multicell enabled
#include "cell.h"
#include "diffusion_op.h"
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
      : backup_(SimulationBackup(Param::backup_file_, Param::restore_file_)),
        grid_(&TGrid::GetInstance()) {
    total_steps_ = 0;
    if (backup_.RestoreEnabled()) {
      restore_point_ = backup_.GetSimulationStepsFromBackup();
    }
    visualization_ = CatalystAdaptor<>::GetInstance();
    if (Param::live_visualization_ || Param::export_visualization_) {
      visualization_->Initialize(BDM_SRC_DIR
                                 "/visualization/simple_pipeline.py");
    }
  }

  virtual ~Scheduler() {
    if (Param::live_visualization_ || Param::export_visualization_) {
      visualization_->Finalize();
    }
    if (Param::statistics_) {
      std::cout << gStatistics << std::endl;
    }
  }

  template <typename Lambda>
  void SimulateTill(Lambda stopping_condition) {
    grid_->Initialize();

    while (!stopping_condition()) {
      // Simulate
      Execute();

      // Visualize
      if (Param::live_visualization_) {
        double time = Param::simulation_time_step_ * total_steps_;
        visualization_->CoProcess(time, total_steps_, false);
      }

      total_steps_++;

      // Backup
      using std::chrono::seconds;
      using std::chrono::duration_cast;
      if (backup_.BackupEnabled() &&
          duration_cast<seconds>(Clock::now() - last_backup_).count() >=
              Param::backup_interval_) {
        last_backup_ = Clock::now();
        backup_.Backup(total_steps_);
      }
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
    if (Param::bound_space_) {
      auto rm = TResourceManager::Get();
      rm->ApplyOnAllTypes(bound_space_);
    }
    int lbound = grid_->GetDimensionThresholds()[0];
    int rbound = grid_->GetDimensionThresholds()[1];
    for (auto& dgrid : TResourceManager::Get()->GetDiffusionGrids()) {
      // Create data structures, whose size depend on the grid dimensions
      dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound},
                        grid_->GetBoxLength());
      // Initialize data structures with user-defined values
      dgrid->RunInitializers();
    }

    for (unsigned step = 0; step < steps; step++) {
      // Visualize
      if (Param::live_visualization_) {
        double time = Param::simulation_time_step_ * total_steps_;
        visualization_->CoProcess(time, total_steps_, step == steps - 1);
      }
      if (Param::export_visualization_) {
        double time = Param::simulation_time_step_ * total_steps_;
        visualization_->ExportVisualization(time, total_steps_,
                                            step == steps - 1);
      }

      // Simulate
      Execute();

      total_steps_++;

      // Backup
      using std::chrono::seconds;
      using std::chrono::duration_cast;
      if (backup_.BackupEnabled() &&
          duration_cast<seconds>(Clock::now() - last_backup_).count() >=
              Param::backup_interval_) {
        last_backup_ = Clock::now();
        backup_.Backup(total_steps_);
      }

      if (Param::show_simulation_step_ &&
          total_steps_ % Param::simulation_step_freq_ == 0) {
        std::cout << "step " << total_steps_ << std::endl;
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
    assert(rm->GetNumSimObjects() > 0 &&
           "This simulation does not contain any simulation objects.");

    {
      if (Param::statistics_) {
        Timing timing("neighbors", &gStatistics);
        grid_->UpdateGrid();
      } else {
        grid_->UpdateGrid();
      }
    }
    // TODO(ahmad): should we only do it here and not after we run the physics?
    // We need it here, because we need to update the threshold values before
    // we update the diffusion grid
    if (Param::bound_space_) {
      rm->ApplyOnAllTypes(bound_space_);
    }
    rm->ApplyOnAllTypes(diffusion_);
    rm->ApplyOnAllTypes(biology_);
    if (Param::run_mechanical_interactions_) {
      rm->ApplyOnAllTypes(physics_with_bound_);
    } else if (Param::bound_space_) {
      rm->ApplyOnAllTypes(bound_space_);
    }
    rm->ApplyOnAllTypes(commit);
  }

 private:
  SimulationBackup backup_;
  size_t& total_steps_ = Param::total_steps_;
  size_t restore_point_;
  std::chrono::time_point<Clock> last_backup_ = Clock::now();
  CatalystAdaptor<>* visualization_ = nullptr;

  OpTimer<DiffusionOp<>> diffusion_ = OpTimer<DiffusionOp<>>("diffusion");
  OpTimer<BiologyModuleOp> biology_ = OpTimer<BiologyModuleOp>("biology");
  OpTimer<DisplacementOp<>> physics_with_bound_ =
      OpTimer<DisplacementOp<>>("physics");
  OpTimer<BoundSpace> bound_space_ = OpTimer<BoundSpace>("bound_space");

  TGrid* grid_;
};

}  // namespace bdm

#endif  // SCHEDULER_H_
