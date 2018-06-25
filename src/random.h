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

#ifndef RANDOM_H_
#define RANDOM_H_

#include <array>
#include <cstdio>

#include <TRandom3.h>
#include <Rtypes.h>

namespace bdm {

/// Decorator for TRandom3
class NewRandom {
 public:
  NewRandom();
  NewRandom& operator=(const NewRandom& other);

  double Uniform(double max = 1.0);
  double Uniform(double min, double max = 1.0);

  template <uint64_t N>
  std::array<double, N> UniformArray(double max = 1.0) {
    std::array<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(0, max);
    }
    return ret;
  }

  template <uint64_t N>
  std::array<double, N> UniformArray(double min, double max = 1.0) {
    std::array<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(min, max);
    }
    return ret;
  }

  double Gaus(double mean = 0.0, double sigma = 1.0);

  void SetSeed(double seed);
  double GetSeed() const;

 private:
  TRandom3 generator_;
  ClassDefNV(NewRandom, 1);
};

// -----------------------------------------------------------------------------


/// C++ implementation of the Java default random number generator
/// (java.util.Random)
class Random {
 public:
  Random() {}

  void SetSeed(int64_t seed);

  int NextInt();

  double NextDouble();

  double NextGaussian(double mean, double standard_deviation);

  std::array<double, 3> NextNoise(double k);

  template <typename Backend>
  std::array<typename Backend::real_v, 3> NextNoise(
      const typename Backend::real_v& k) {
    std::array<typename Backend::real_v, 3> ret;
    for (size_t i = 0; i < Backend::kVecLen; i++) {
      // todo not most cache friendly way
      ret[0][i] = -k[i] + 2 * k[i] * NextDouble();
      ret[1][i] = -k[i] + 2 * k[i] * NextDouble();
      ret[2][i] = -k[i] + 2 * k[i] * NextDouble();
    }
    return ret;
  }

 private:
  int64_t seed_ = 0;
  double next_next_gaussian_ = 0.0;
  bool have_next_next_gaussian_ = false;

  int Next(int i);

  double NextGaussian();

  bool CompareAndSet(int64_t* current, int64_t expected, int64_t update);
};

extern thread_local Random gRandom;




}  // namespace bdm

#endif  // RANDOM_H_
