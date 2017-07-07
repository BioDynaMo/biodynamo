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

  inline double SquaredEuclideanDistance(std::array<double, 3> pos1, std::array<double, 3> pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dy = pos2[1] - pos1[1];
    const double dz = pos2[2] - pos1[2];
    return (dx * dx + dy * dy + dz * dz);
  }

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    // Construct a 3D grid with the current positions for the simulation objects
    Grid grid(cells, 30);

//    // Define the lambda that fills a vector of neighbors for each cell
//    // within a set distance (excluding itself)
//    auto search_radius = [&] (const Grid::NeighborIterator& ni, size_t qc, bool is_begin, bool is_end) {
//      // nc = neighbor cell
//      size_t nc = *ni;
//      InlineVector<int, 8>* neighbors = nullptr;
//      if (is_begin) { neighbors = new InlineVector<int, 8>(); }
//      if (is_end) {
//        (*cells)[qc].SetNeighbors(*neighbors);
//        delete neighbors;
//      } else if (nc != qc) {
//        if (SquaredEuclideanDistance(positions[qc], positions[nc]) < distance_) {
//          neighbors->push_back(nc);
//        }
//      }
//    };

    // Initiate the operation
    grid.SetNeighborsWithinRadius(cells, distance_);
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_GRID_OP_H_
