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

#ifndef CORE_ANALYSIS_MEAN_SQUARED_ERROR_H_
#define CORE_ANALYSIS_MEAN_SQUARED_ERROR_H_

#include <vector>

#include "core/analysis/error_computation/error_computation.h"
#include "core/util/log.h"

namespace bdm {

struct MeanSquaredError : public ErrorComputation {
  double operator()(const std::vector<double>& a,
                    const std::vector<double>& b) override {
    if (a.size() != b.size()) {
      Log::Error("MeanSquaredError", "Dimension mismatch between inputs.");
    }

    // TODO: implement parallel reduction
    double error = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
      error += std::pow(a[i] - b[i], 2);
    }

    return error;
  }
};

}  // namespace bdm

#endif  // CORE_ANALYSIS_MEAN_SQUARED_ERROR_H_
