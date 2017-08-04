#ifndef NEIGHBOR_GRID_OP_H_
#define NEIGHBOR_GRID_OP_H_

#include <utility>
#include <vector>

#include "grid.h"
#include "inline_vector.h"

namespace bdm {

/// A class that sets up an uniform grid to perform operations that require
/// knowledge about neighboring simulation objects
class NeighborGridOp {
 public:
  explicit NeighborGridOp(Grid::Adjacency adjacency = Grid::kHigh,
                          bool set_local_neighbors = false,
                          double radius = 3000)
      : adjacency_(adjacency),
        radius_(radius) {}
  virtual ~NeighborGridOp() {}

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    // Construct a 3D grid with the current positions for the simulation objects
    auto& grid = Grid::GetInstance();
    grid.Initialize(cells, adjacency_);
  }

 private:
  /// Determines how many neighboring boxes to consider for neighbor operations
  Grid::Adjacency adjacency_;
  /// The searching radius for which to set the local neighbors to
  double radius_;
};

}  // namespace bdm

#endif  // NEIGHBOR_GRID_OP_H_
