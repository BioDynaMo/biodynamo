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

#ifndef CORE_UTIL_RANDOM_H_
#define CORE_UTIL_RANDOM_H_

#include <array>
#include <cstdio>

#include <TRandom3.h>
#include "core/container/math_array.h"
#include "core/util/root.h"

namespace bdm {

/// Decorator for ROOT's TRandom3
class Random {
 public:
  Random();
  Random& operator=(const Random& other);

  /// Forwards call to TRandom3::Uniform
  double Uniform(double max = 1.0);
  /// Forwards call to TRandom3::Uniform
  double Uniform(double min, double max);

  /// Returns an array of uniform random numbers in the interval (0, max)
  template <uint64_t N>
  MathArray<double, N> UniformArray(double max = 1.0) {
    MathArray<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(max);
    }
    return ret;
  }

  /// Returns an array of uniform random numbers in the interval (min, max)
  template <uint64_t N>
  MathArray<double, N> UniformArray(double min, double max) {
    MathArray<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(min, max);
    }
    return ret;
  }

  /// Forwards call to TRandom3::Gaus
  double Gaus(double mean = 0.0, double sigma = 1.0);

  /// Forwards call to TRandom3::SetSeed
  void SetSeed(double seed);

  /// Forwards call to TRandom3::GetSeed
  double GetSeed() const;

 private:
  TRandom3 generator_;
  BDM_CLASS_DEF_NV(Random, 1);
};

}  // namespace bdm

#endif  // CORE_UTIL_RANDOM_H_
