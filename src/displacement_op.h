#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include "displacement_op_cpu.h"
#ifdef USE_CUDA
#include "displacement_op_cuda.h"
#endif
#ifdef USE_OPENCL
#include "displacement_op_opencl.h"
#endif
#include "grid.h"
#include "param.h"

namespace bdm {

using std::array;

template <typename TSO>
void ApplyBoundingBox(TSO* cell, double lb, double rb) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  double eps = 1e-10;
  auto& pos = cell->GetPosition();
  for (int i = 0; i < 3; i++) {
    if (pos[i] < lb) {
      cell->SetCoordinate(i, lb);
    }
    if (pos[i] >= rb) {
      cell->SetCoordinate(i, rb - eps);
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
  void operator()(TContainer* cells, uint16_t type_idx) {
    if (Param::use_gpu_) {
#ifdef USE_OPENCL
      if (Param::use_opencl_) {
        opencl_(cells, type_idx);
      }
#endif
#ifdef USE_CUDA
      else {
        cuda_(cells, type_idx);
      }
#endif
    } else {
      cpu_(cells, type_idx);
    }
  }

 private:
  DisplacementOpCpu<TGrid> cpu_;
#ifdef USE_CUDA
  DisplacementOpCuda<TGrid> cuda_;  // NOLINT
#endif
#ifdef USE_OPENCL
  DisplacementOpOpenCL<TGrid> opencl_;
#endif
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
    auto& grid = Grid<>::GetInstance();
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      if (Param::bound_space_) {
        ApplyBoundingBox(&cell, Param::min_bound_, Param::max_bound_);
        grid.SetDimensionThresholds(Param::min_bound_, Param::max_bound_);
      }
      cell.SetPosition(cell.GetMassLocation());

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
