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

#ifndef CORE_MULTI_SIMULATION_ERROR_MATRIX_H_
#define CORE_MULTI_SIMULATION_ERROR_MATRIX_H_

#include <cmath>
#include <iostream>
#include <vector>

namespace bdm {

class ErrorMatrix {
 public:
  virtual ~ErrorMatrix() {}

  /// Compute the error between `real` and `simulated`
  virtual float Compute(float real, float simulated) = 0;

  /// Register the raw values that lead to the computed error value
  void RegisterError(float error) { error_history_.push_back(error); }

  void Print() {
    std::cout << "Error history:" << std::endl;
    for (const auto& entry : error_history_) {
      std::cout << entry << std::endl;
    }
  }

 private:
  std::vector<float> error_history_;
};

class SquaredError : public ErrorMatrix {
 public:
  float Compute(float real, float simulated) override {
    float error = std::pow(real - simulated, 2);
    RegisterError(error);
    return error;
  }
};

class AbsoluteError : public ErrorMatrix {
 public:
  float Compute(float real, float simulated) override {
    float error = std::abs(real - simulated);
    RegisterError(error);
    return error;
  }
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_ERROR_MATRIX_H_
