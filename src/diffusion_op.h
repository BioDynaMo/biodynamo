#ifndef DIFFUSION_OP_H_
#define DIFFUSION_OP_H_

#include <string>
#include <utility>
#include <vector>

#include "cell.h"
#include "diffusion_grid.h"
#include "grid.h"
#include "inline_vector.h"
#include "resource_manager.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
template <typename TResourceManager = ResourceManager<>>
class DiffusionOp {
 public:
  DiffusionOp() {}
  virtual ~DiffusionOp() {}

  template <typename TContainer, typename TGrid = Grid<>>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto& grid = TGrid::GetInstance();
    auto& diffusion_grids = TResourceManager::Get()->GetDiffusionGrids();
    for (auto dg : diffusion_grids) {
      // Update the diffusion grid dimension if the neighbor grid
      // dimensions have changed
      if (grid.HasGrown()) {
        std::cout << "Your simulation objects are getting near the edge of the "
                     "simulation space. Be aware of boundary conditions that may "
                     "come into play!"
                  << std::endl;
        dg->Update(grid.GetDimensionThresholds());
      }

      // dg->DiffuseWithClosedEdge();
      dg->DiffuseWithLeakingEdge();
      dg->CalculateGradient();
    }
  }
};

}  // namespace bdm

#endif  // DIFFUSION_OP_H_
