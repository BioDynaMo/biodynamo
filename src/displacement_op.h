#ifndef DISPLACEMENT_OP_H_
#define DISPLACEMENT_OP_H_

#include <type_traits>

#include "displacement_op_cpu.h"
#ifdef USE_CUDA
#include "displacement_op_cuda.h"
#endif
#ifdef USE_OPENCL
#include "displacement_op_opencl.h"
#endif
#include "grid.h"
#include "log.h"
#include "param.h"
#include "shape.h"

namespace bdm {

using std::array;

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

/// Defines the 3D physical interactions between physical objects
template <typename TRm = ResourceManager<>>
class DisplacementOp {
 public:
  DisplacementOp() {
    TRm::Get()->template ApplyOnAllTypes([this](auto* container, uint16_t type_idx) {
      using Container = std::remove_pointer_t<decltype(container)>;
      using SimObject = typename Container::value_type;
      if (SimObject::GetShape() != Shape::kSphere) {
        Log::Warning("DisplacementOp", "Currently GPU implementation only supports Spheres shapes. Therefore, the CPU implementation will be used.");
        this->force_cpu_implementation_ = true;
      }
    });
  }

  ~DisplacementOp() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) {
    if (Param::use_gpu_ && !force_cpu_implementation_) {
#ifdef USE_OPENCL
      if (Param::use_opencl_) {
        opencl_(cells, type_idx);
      }
#endif
#ifdef USE_CUDA
      if (!Param::use_opencl_) {
        cuda_(cells, type_idx);
      }
#endif
    } else {
      cpu_(cells, type_idx);
    }
  }

 private:
  /// Currently the gpu implementation only supports Spheres.
  /// If a simulation contains simulation objects with different shapes with
  /// GPU turned on, this wouldn't work. The CPU implementation should be used
  /// if this condition is detected. In this case `force_cpu_implementation_`
  /// will be set to true.
  bool force_cpu_implementation_ = false;
  DisplacementOpCpu<> cpu_;
#ifdef USE_CUDA
  DisplacementOpCuda<> cuda_;  // NOLINT
#endif
#ifdef USE_OPENCL
  DisplacementOpOpenCL<> opencl_;
#endif
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

#endif  // DISPLACEMENT_OP_H_
