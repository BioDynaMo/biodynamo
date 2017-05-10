#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "neighbor_op.h"
#include "resource_manager.h"

namespace bdm {

class Scheduler {
 public:
  Scheduler() {}

  virtual ~Scheduler() {}

  template <typename Backend>
  void Simulate(unsigned steps) {
    auto rm = ResourceManager<Backend>::Get();
    DisplacementOp physics;
    DividingCellOp biology;
    NeighborOp neighbor;
    while (steps-- > 0) {
      auto cells = rm->GetCells();  // todo why does this compile GetCells
                                    // returns const?!?
      physics.Compute(&cells);
      // todo transfrom cells based on opdefinition
      biology.Compute(&cells);
      neighbor.Compute(&cells);
      rm->SetCells(cells);
    }
  }
};

}  // namespace bdm

#endif  // SCHEDULER_H_
