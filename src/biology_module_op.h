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

#ifndef BIOLOGY_MODULE_OP_H_
#define BIOLOGY_MODULE_OP_H_

#include <cstddef>  // std::size_t
#include <cstdint>  // uint16_t

#include "debug.h"

namespace bdm {

struct BiologyModuleOp {
  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
#pragma omp parallel
    {
// Iterations are data independent, so threads don't need to
// wait for each other, which can improve performance when
// biology modules are not equal in workload
#pragma omp for nowait
      for (uint64_t i = 0; i < cells->size(); i++) {
        (*cells)[i].RunBiologyModules();
      }
    }
  }
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_OP_H_
