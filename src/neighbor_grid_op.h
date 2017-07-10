#ifndef NEIGHBOR_GRID_OP_H_
#define NEIGHBOR_GRID_OP_H_

#include <utility>
#include <vector>

#include "grid.h"
#include "inline_vector.h"

namespace bdm {

class NeighborGridOp {
 public:
  NeighborGridOp() {}
  explicit NeighborGridOp(double distance) : distance_(distance) {}
  ~NeighborGridOp() {}

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    // Construct a 3D grid with the current positions for the simulation objects
    // NB: the box size needs to be at least as big as the search radius
    const double box_size = 55;
    Grid grid(cells, box_size, Grid::HIGH);

    // Initiate the operation
    grid.SetNeighborsWithinRadius(cells, distance_);
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_GRID_OP_H_
