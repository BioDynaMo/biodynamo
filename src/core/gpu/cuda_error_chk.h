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

#ifndef CORE_GPU_CUDA_ERROR_CHK_H_
#define CORE_GPU_CUDA_ERROR_CHK_H_

#if !defined(__ROOTCLING__) && !defined(G__DICTIONARY)
#ifdef USE_CUDA

#include <unistd.h>
#include <cstdio>

#define GpuErrchk(ans) \
  { GpuAssert((ans), __FILE__, __LINE__); }
inline void GpuAssert(cudaError_t code, const char *file, int line,
                      bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(stderr, "GPUassert (error code %d): %s %s %d\n", code,
            cudaGetErrorString(code), file, line);
    if (code == cudaErrorInsufficientDriver) {
      printf(
          "This probably means that no CUDA-compatible GPU has been detected. "
          "Consider setting the use_opencl flag to \"true\" in the bmd.toml "
          "file to use OpenCL instead.\n");
    }
    if (abort)
      exit(code);
  }
}

#endif  // USE_CUDA
#endif  // !__ROOTCLING__ && !G__DICTIONARY
#endif  // CORE_GPU_CUDA_ERROR_CHK_H_
