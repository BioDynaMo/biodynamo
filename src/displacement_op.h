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
void ApplyBoundingBox(TSO* sim_object, double lb, double rb) {
  auto pos = sim_object->GetPosition();
  for (int i = 0; i < 3; i++) {
    if (pos[i] < lb) {
      pos[i] = lb;
    }
    else if (pos[i] > rb) {
      pos[i] = rb;
    }
  }
  sim_object->SetPosition(pos);
}

/// Defines the 3D physical interactions between physical objects
template <typename TGrid = Grid<>>
class DisplacementOp {
 public:
  DisplacementOp() {}
  ~DisplacementOp() {}

  template <typename TContainer>
  void operator()(TContainer* sim_objects, uint16_t type_idx) const {
    std::vector<array<double, 3>> sim_object_movements;
    sim_object_movements.reserve(sim_objects->size());

    auto& grid = TGrid::GetInstance();
    auto search_radius = grid.GetLargestObjectSize();
    double squared_radius = search_radius * search_radius;

#pragma omp parallel for shared(grid) firstprivate(squared_radius)
    for (size_t i = 0; i < sim_objects->size(); i++) {
      sim_object_movements[i] = (*sim_objects)[i].CalculateDisplacement(&grid, squared_radius);
    }

// set new positions after all updates have been calculated
// otherwise some sim_objects would see neighbors with already updated positions
// which would lead to inconsistencies
// FIXME there are still inconsistencies if there are more than one simulation
//  object types!
#pragma omp parallel for
    for (size_t i = 0; i < sim_objects->size(); i++) {
      auto&& sim_object = (*sim_objects)[i];
      sim_object.ApplyDisplacement(sim_object_movements[i]);
      if (Param::bound_space_) {
        ApplyBoundingBox(&sim_object, Param::min_bound_, Param::max_bound_);
      }
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
  void operator()(TContainer* sim_objects, uint16_t type_idx) const {
// set new positions after all updates have been calculated
// otherwise some sim_objects would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < sim_objects->size(); i++) {
      auto&& sim_object = (*sim_objects)[i];
      if (Param::bound_space_) {
        ApplyBoundingBox(&sim_object, Param::min_bound_, Param::max_bound_);
      }
    }
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
