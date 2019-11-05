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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_H_
#define CORE_OPERATION_DISPLACEMENT_OP_H_

#include <type_traits>

#include "core/operation/displacement_op_cpu.h"
#ifdef USE_CUDA
#include "core/operation/displacement_op_cuda.h"
#endif
#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
#include "core/operation/displacement_op_opencl.h"
#endif
#include "core/grid.h"
#include "core/param/param.h"
#include "core/scheduler.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
class DisplacementOp {
 public:
  DisplacementOp() {
    // NB: check if there are non spherical shapes is not easily possible in the
    // dynamic solution.
  }

  ~DisplacementOp() {}

  bool UseCpu() const {
    auto* param = Simulation::GetActive()->GetParam();
    return force_cpu_implementation_ ||
           (!param->use_gpu_ && !param->use_opencl_);
  }

  void operator()() {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->use_gpu_ && !force_cpu_implementation_) {
#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
      if (param->use_opencl_) {
        opencl_();
      }
#endif
#ifdef USE_CUDA
      if (!param->use_opencl_) {
        cuda_();
      }
#endif
    }
  }

  void operator()(SimObject* sim_object) { cpu_(sim_object); }

 private:
  /// Currently the gpu implementation only supports Spheres.
  /// If a simulation contains simulation objects with different shapes with
  /// GPU turned on, this wouldn't work. The CPU implementation should be used
  /// if this condition is detected. In this case `force_cpu_implementation_`
  /// will be set to true.
  bool force_cpu_implementation_ = false;
  DisplacementOpCpu cpu_;
#ifdef USE_CUDA
  DisplacementOpCuda cuda_;  // NOLINT
#endif
#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
  DisplacementOpOpenCL opencl_;
#endif
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_H_
