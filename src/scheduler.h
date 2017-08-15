#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "biology_module_op.h"
#include "cell.h"
#include "displacement_op.h"
#include "op_timer.h"
#include "resource_manager.h"

namespace bdm {

class Scheduler {
 public:
  Scheduler() {}

  virtual ~Scheduler() {}

  template <typename TResourceManager = ResourceManager<>,
            typename TGrid = Grid<>>
  void Simulate(unsigned steps) {
    OpTimer<BiologyModuleOp> biology("biology");
    OpTimer<DisplacementOp<>> physics("physics");

    auto rm = TResourceManager::Get();
    auto commit = [](auto* sim_objects, uint16_t type_idx) {
      sim_objects->Commit();
    };
    auto& grid = TGrid::GetInstance();
    grid.Initialize();

    while (steps-- > 0) {
      {
        Timing timing("neighbors");
        grid.UpdateGrid();
      }
      rm->ApplyOnAllTypes(biology);
      rm->ApplyOnAllTypes(physics);
      rm->ApplyOnAllTypes(commit);
    }
  }
};

}  // namespace bdm

#endif  // SCHEDULER_H_
