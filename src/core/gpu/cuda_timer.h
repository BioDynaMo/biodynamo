// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_GPU_CUDA_TIMER_H_
#define CORE_GPU_CUDA_TIMER_H_

#if !defined(__ROOTCLING__) && !defined(G__DICTIONARY)
#ifdef USE_CUDA

#include <iostream>
#include <string>

namespace bdm {

class CudaTimer {
 public:
  CudaTimer(std::string name) : name_(name) {
    cudaEventCreate(&start_);
    cudaEventCreate(&stop_);
    cudaEventRecord(start_, 0);
  }

  ~CudaTimer() {
    cudaEventRecord(stop_, 0);
    float duration;
    cudaEventSynchronize(stop_);
    cudaEventElapsedTime(&duration, start_, stop_);
    std::cout << name_ << " " << duration << std::endl;
    cudaEventDestroy(start_);
    cudaEventDestroy(stop_);
  }

 private:
  cudaEvent_t start_;
  cudaEvent_t stop_;
  std::string name_;
};

}  // namespace bdm

#endif  // USE_CUDA
#endif  // !__ROOTCLING__ && !G__DICTIONARY
#endif  // CORE_GPU_CUDA_TIMER_H_
