#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include <array>
#include <cmath>
#include <vector>
#include "grid.h"
#include "math_util.h"
#include "param.h"

namespace bdm {

using std::array;

template <typename TSO>
void ApplyBoundingBox(TSO* cell, double lb, double rb) {
  auto& pos = cell->GetPosition();
  for (int i = 0; i < 3; i++) {
    if (pos[i] < lb) {
      cell->SetCoordinate(i, lb);
    }
    if (pos[i] > rb) {
      cell->SetCoordinate(i, rb);
    }
  }
}

/// Defines the 3D physical interactions between physical objects
template <typename TGrid = Grid<>>
class DisplacementOp {
 public:
  DisplacementOp() {}
  ~DisplacementOp() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
    std::vector<array<double, 3>> cell_movements;
    cell_movements.reserve(cells->size());

    auto& grid = TGrid::GetInstance();
    auto search_radius = grid.GetLargestObjectSize();
    double squared_radius = search_radius * search_radius;

#pragma omp parallel for shared(grid) firstprivate(squared_radius)
    for (size_t i = 0; i < cells->size(); i++) {
      cell_movements[i] = (*cells)[i].RunPhysics(&grid, squared_radius);
    }

// set new positions after all updates have been calculated
// otherwise some cells would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      cell.UpdatePosition(cell_movements[i]);
      if (Param::bound_space_) {
        ApplyBoundingBox(&cell, Param::min_bound_, Param::max_bound_);
      }

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
  }
};

/// Keeps the simulation objects contained within the bounds as defined in
/// param.h
class BoundSpace {
 public:
  BoundSpace() {}
  ~BoundSpace() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
// set new positions after all updates have been calculated
// otherwise some cells would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      if (Param::bound_space_) {
        ApplyBoundingBox(&cell, Param::min_bound_, Param::max_bound_);
      }

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
