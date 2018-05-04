#ifndef BOUND_SPACE_OP_H_
#define BOUND_SPACE_OP_H_

#include "grid.h"

namespace bdm {

template <typename TSO>
void ApplyBoundingBox(TSO* sim_object, double lb, double rb) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  double eps = 1e-10;
  auto pos = sim_object->GetPosition();
  for (int i = 0; i < 3; i++) {
    if (pos[i] < lb) {
      pos[i] = lb;
    } else if (pos[i] >= rb) {
      pos[i] = rb - eps;
    }
  }
  sim_object->SetPosition(pos);
}

/// Keeps the simulation objects contained within the bounds as defined in
/// param.h
class BoundSpace {
 public:
  BoundSpace() {}
  ~BoundSpace() {}

  template <typename TContainer>
  void operator()(TContainer* sim_objects, uint16_t type_idx) const {
    // set new positions after all updates have been calculated
    // otherwise some sim_objects would see neighbors with already updated
    // positions
    // which would lead to inconsistencies
    auto& grid = Grid<>::GetInstance();
#pragma omp parallel for
    for (size_t i = 0; i < sim_objects->size(); i++) {
      auto&& sim_object = (*sim_objects)[i];
      if (Param::bound_space_) {
        ApplyBoundingBox(&sim_object, Param::min_bound_, Param::max_bound_);
        grid.SetDimensionThresholds(Param::min_bound_, Param::max_bound_);
      }
    }
  }
};

}  // namespace bdm

#endif  // BOUND_SPACE_OP_H_
