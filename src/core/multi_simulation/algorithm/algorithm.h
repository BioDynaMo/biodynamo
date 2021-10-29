// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_
#define CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_

#include <functional>

#include "core/analysis/time_series.h"
#include "core/functor.h"
#include "core/param/param.h"

namespace bdm {
namespace experimental {

using experimental::TimeSeries;

/// An interface for creating new optimization algorithms
struct Algorithm {
  virtual ~Algorithm() {}

  virtual void operator()(
      Functor<void, Param*, TimeSeries*>& dispatch_experiment,
      Param* default_param) = 0;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_
