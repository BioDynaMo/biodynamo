// //
// -----------------------------------------------------------------------------
// //
// // Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// // BioDynaMo collaboration. All Rights Reserved.
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// //
// // See the LICENSE file distributed with this work for details.
// // See the NOTICE file distributed with this work for additional information
// // regarding copyright ownership.
// //
// //
// -----------------------------------------------------------------------------

#ifndef CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_
#define CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_

#include "core/gpu/mechanical_forces_op_cuda_kernel.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"

namespace bdm {

namespace detail {

struct InitializeGPUData;

}  // namespace detail

/// Defines the 3D physical interactions between physical objects
struct MechanicalForcesOpCuda : public StandaloneOperationImpl {
  BDM_OP_HEADER(MechanicalForcesOpCuda);

 public:
  void SetUp() override;

  void operator()() override;

  void TearDown() override;

 private:
  MechanicalForcesOpCudaKernel* cdo_ = nullptr;
  detail::InitializeGPUData* i_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t total_num_agents_ = 0;
};

}  // namespace bdm

#endif  // CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_
