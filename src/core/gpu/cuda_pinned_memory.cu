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

#include "core/gpu/cuda_pinned_memory.h"

namespace bdm {

template <typename T>
void AllocPinned(T** d, uint64_t elements) {
  cudaMallocHost((void**)d, elements * sizeof(T));
}

template void AllocPinned<double>(double**, uint64_t);
template void AllocPinned<float>(float**, uint64_t);
template void AllocPinned<uint64_t>(uint64_t**, uint64_t);
template void AllocPinned<int64_t>(int64_t**, uint64_t);
template void AllocPinned<uint32_t>(uint32_t**, uint64_t);
template void AllocPinned<int32_t>(int32_t**, uint64_t);
template void AllocPinned<uint16_t>(uint16_t**, uint64_t);
template void AllocPinned<int16_t>(int16_t**, uint64_t);

}  // namespace bdm

