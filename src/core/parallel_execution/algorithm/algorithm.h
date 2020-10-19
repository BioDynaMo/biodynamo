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

#ifndef CORE_PARALLEL_EXECUTION_ALGORITHM_ALGORITHM_H_
#define CORE_PARALLEL_EXECUTION_ALGORITHM_ALGORITHM_H_

#include <functional>
#include <string>

#include "core/parallel_execution/optimization_param.h"
#include "core/param/param.h"

namespace bdm {

struct Algorithm {
  virtual ~Algorithm() {}

  virtual void operator()(
      const std::function<void(Param*)>& send_params_to_worker) = 0;

  OptimizationParam* opt_params_;
  Param* default_params_;
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_ALGORITHM_ALGORITHM_H_
