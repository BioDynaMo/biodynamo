// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_GPU_GPU_HELPER_H_
#define CORE_GPU_GPU_HELPER_H_

namespace bdm {

class GpuHelper {
 public:
  static GpuHelper* GetInstance();

#ifdef USE_CUDA
  static void FindGpuDevicesCuda();
#endif  // USE_CUDA

#ifdef USE_OPENCL
  void CompileOpenCLKernels();

  void FindGpuDevicesOpenCL();
#endif  // defined(USE_OPENCL)

  void InitializeGPUEnvironment();
};

}  // namespace bdm

#endif  // CORE_GPU_GPU_HELPER_H_
