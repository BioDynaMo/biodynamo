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

#include "random.h"

#include <cmath>
#include <random>

namespace bdm {

NewRandom::NewRandom() {}

NewRandom& NewRandom::operator=(const NewRandom& other) {
  generator_ = other.generator_;
  return *this;
}

double NewRandom::Uniform(double max) { return generator_.Uniform(max); }
double NewRandom::Uniform(double min, double max) {
  return generator_.Uniform(min, max);
}

double NewRandom::Gaus(double mean, double sigma) {
  return generator_.Gaus(mean, sigma);
}

void NewRandom::SetSeed(double seed) { generator_.SetSeed(seed); }
double NewRandom::GetSeed() const { return generator_.GetSeed(); }


// -----------------------------------------------------------------------------

thread_local Random gRandom;

void Random::SetSeed(int64_t seed) {
  seed_ = (seed ^ 25214903917L) & 281474976710655L;
  next_next_gaussian_ = 0.0;
  have_next_next_gaussian_ = false;
}

int Random::NextInt() { return std::rand(); }

double Random::NextDouble() {
  return static_cast<double>((static_cast<int64_t>(Next(26)) << 27) +
                             static_cast<int64_t>(Next(27))) *
         1.1102230246251565E-16;
}

double Random::NextGaussian(double mean, double standard_deviation) {
  return mean + standard_deviation * NextGaussian();
}

int Random::Next(int var1) {
  int64_t var6 = seed_;

  int64_t var2;
  int64_t var4;
  do {
    var2 = var6;
    var4 = (var2 * 25214903917L + 11L) & 281474976710655L;
  } while (!CompareAndSet(&var6, var2, var4));
  seed_ = var6;
  return static_cast<int>(var4 >> (48 - var1));
}

bool Random::CompareAndSet(int64_t* current, int64_t expected, int64_t update) {
  if (*current == expected) {
    *current = update;
    return true;
  }
  return false;
}

double Random::NextGaussian() {
  if (have_next_next_gaussian_) {
    have_next_next_gaussian_ = false;
    return next_next_gaussian_;
  } else {
    double v1, v2, s;
    do {
      v1 = 2 * NextDouble() - 1;  // between -1.0 and 1.0
      v2 = 2 * NextDouble() - 1;  // between -1.0 and 1.0
      s = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);
    double multiplier = std::sqrt(-2 * std::log(s) / s);
    next_next_gaussian_ = v2 * multiplier;
    have_next_next_gaussian_ = true;
    return v1 * multiplier;
  }
}

std::array<double, 3> Random::NextNoise(double k) {
  std::array<double, 3> ret;
  ret[0] = -k + 2 * k * NextDouble();
  ret[1] = -k + 2 * k * NextDouble();
  ret[2] = -k + 2 * k * NextDouble();
  return ret;
}

}  // namespace bdm
