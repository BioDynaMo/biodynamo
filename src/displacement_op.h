// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

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

#include "simulation.h"

namespace bdm {

using std::array;

/// Defines the 3D physical interactions between physical objects
template <typename TSimulation = Simulation<>>
class DisplacementOp {
 public:
  DisplacementOp() {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    rm->template ApplyOnAllTypes([this](auto* container,
                                                uint16_t type_idx) {
      using Container = std::remove_pointer_t<decltype(container)>;
      using SimObject = typename Container::value_type;
      if (SimObject::GetShape() != Shape::kSphere) {
        Log::Warning("DisplacementOp",
                     "Currently GPU implementation only supports Spheres "
                     "shapes. Therefore, the CPU implementation will be used.");
        this->force_cpu_implementation_ = true;
      }
    });
  }

  ~DisplacementOp() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto* param = TSimulation::GetActive()->GetParam();
    if (param->use_gpu_ && !force_cpu_implementation_) {
#ifdef USE_OPENCL
      if (param->use_opencl_) {
        opencl_(cells, type_idx);
      }
#endif
#ifdef USE_CUDA
      if (!param->use_opencl_) {
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

}  // namespace bdm

#endif  // DISPLACEMENT_OP_H_
