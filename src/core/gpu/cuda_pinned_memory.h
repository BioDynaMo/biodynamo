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

#ifndef CORE_GPU_CUDA_PINNED_MEMORY_H_
#define CORE_GPU_CUDA_PINNED_MEMORY_H_

#include <cstdint>

namespace bdm {

#ifdef USE_CUDA

/// Allocate number of element T objects on host pinned memory 
template <typename T>
void CudaAllocPinned(T** d, uint64_t elements);

void CudaFreePinned(void* p);

#else

template <typename T>
void CudaAllocPinned(T** d, uint64_t elements) {}

void CudaFreePinned(void* p) {}

#endif  // USE_CUDA

}  // namespace bdm

#endif  // CORE_GPU_CUDA_PINNED_MEMORY_H_
