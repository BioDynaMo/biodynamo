#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "biology_module_op.h"
#include "displacement_op.h"
#include "displacement_op_new.h"
#include "neighbor_grid_op.h"
#include "op_timer.h"
#include "resource_manager.h"

namespace bdm {

class Scheduler {
 public:
  Scheduler() {}

  virtual ~Scheduler() {}

  template <typename TCellContainer>
  void Simulate(unsigned steps) {
    OpTimer<NeighborGridOp> neighbor("neighbor", NeighborGridOp());
    // OpTimer<BiologyModuleOp> biology("biology");
    OpTimer<DisplacementOp> physics("physics    ");
    OpTimer<DisplacementOpNew> physics_new("physics new");

    auto cells = ResourceManager<TCellContainer>::Get()->GetCells();

    while (steps-- > 0) {
      neighbor.Compute(cells);
      // biology.Compute(cells);
      physics.Compute(cells);
      physics_new.Compute(cells);
      // cells->Commit();
    }
  }
};

}  // namespace bdm

#endif  // SCHEDULER_H_
