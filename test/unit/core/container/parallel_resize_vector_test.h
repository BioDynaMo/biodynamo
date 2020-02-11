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

#ifndef UNIT_CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_TEST_H_
#define UNIT_CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_TEST_H_

#include "core/container/parallel_resize_vector.h"

namespace bdm {

// For genreflex to generate appropriate dictionaries
#ifdef __ROOTCLING__
static ParallelResizeVector<unsigned> prvu;
static ParallelResizeVector<int> prvi;
#endif

}  // namespace bdm

#endif  // UNIT_CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_TEST_H_
