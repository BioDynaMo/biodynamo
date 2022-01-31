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

#include "core/gpu/cuda_pinned_memory.h"
#include "core/gpu/cuda_error_chk.h"

namespace bdm {

template <typename T>
void CudaAllocPinned(T** d, uint64_t elements) {
  GpuErrchk(cudaMallocHost((void**)d, elements * sizeof(T)));
}

template void CudaAllocPinned<double>(double**, uint64_t);
template void CudaAllocPinned<float>(float**, uint64_t);
template void CudaAllocPinned<uint64_t>(uint64_t**, uint64_t);
template void CudaAllocPinned<int64_t>(int64_t**, uint64_t);
template void CudaAllocPinned<uint32_t>(uint32_t**, uint64_t);
template void CudaAllocPinned<int32_t>(int32_t**, uint64_t);
template void CudaAllocPinned<uint16_t>(uint16_t**, uint64_t);
template void CudaAllocPinned<int16_t>(int16_t**, uint64_t);

void CudaFreePinned(void* p) {
  GpuErrchk(cudaFreeHost(p));
}

}  // namespace bdm
