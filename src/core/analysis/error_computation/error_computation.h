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

#ifndef CORE_ANALYSIS_ERROR_COMPUTATION_H_
#define CORE_ANALYSIS_ERROR_COMPUTATION_H_

#include <vector>

namespace bdm {

/// Base class that represents the results of a single experiment
struct ErrorComputation {
  ErrorComputation() {}

  virtual ~ErrorComputation() {}

  virtual double operator()(const std::vector<double>& a,
                            const std::vector<double>& b);
};

}  // namespace bdm

#endif  // CORE_ANALYSIS_ERROR_COMPUTATION_H_
