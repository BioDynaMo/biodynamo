#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "biology_module_op.h"
#include "displacement_op.h"
#include "neighbor_nanoflann_op.h"
#include "op_timer.h"
#include "resource_manager.h"

#ifdef USE_CATALYST
#include "param.h"
#include "visualization/catalyst_adaptor.h"
#endif

namespace bdm {

class Scheduler {
 public:
  Scheduler() {
#ifdef USE_CATALYST
    catalyst::Initialize("../src/visualization/simple_pipeline.py");
#endif
  }

  virtual ~Scheduler() {
#ifdef USE_CATALYST
    catalyst::Finalize();
#endif
  }

  template <typename TCellContainer>
  void Simulate(unsigned steps) {
    OpTimer<NeighborNanoflannOp> neighbor("neighbor", NeighborNanoflannOp(700));
    OpTimer<BiologyModuleOp> biology("biology");
    OpTimer<DisplacementOp> physics("physics");

    auto cells = ResourceManager<TCellContainer>::Get()->GetCells();

    for (unsigned step = 0; step < steps; step++) {
      neighbor.Compute(cells);
      biology.Compute(cells);
      physics.Compute(cells);
      cells->Commit();

#ifdef USE_CATALYST
      double time = Param::kSimulationTimeStep * total_steps;
      catalyst::CoProcess(cells, time, total_steps, step == steps - 1);
#endif

      total_steps++;
    }
  }

 private:
  unsigned long total_steps = 0;
};

}  // namespace bdm

#endif  // SCHEDULER_H_
