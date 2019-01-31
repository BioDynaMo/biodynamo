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

#include "core/util/random.h"

#include <cmath>
#include <random>

namespace bdm {

Random::Random() {}

Random& Random::operator=(const Random& other) {
  generator_ = other.generator_;
  return *this;
}

double Random::Uniform(double max) { return generator_.Uniform(max); }
double Random::Uniform(double min, double max) {
  return generator_.Uniform(min, max);
}

double Random::Gaus(double mean, double sigma) {
  return generator_.Gaus(mean, sigma);
}

void Random::SetSeed(double seed) { generator_.SetSeed(seed); }
double Random::GetSeed() const { return generator_.GetSeed(); }

}  // namespace bdm
