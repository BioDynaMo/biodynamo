// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/util/random.h"
#include <TF1.h>
#include <TF2.h>
#include <TF3.h>
#include <TRandom3.h>
#include "core/simulation.h"

namespace bdm {

// -----------------------------------------------------------------------------
Random::Random() : generator_(new TRandom3()) {}

// -----------------------------------------------------------------------------
Random::Random(TRootIOCtor*) {}

// -----------------------------------------------------------------------------
Random::Random(const Random& other)
    : generator_(static_cast<TRandom*>(other.generator_->Clone())) {}

// -----------------------------------------------------------------------------
Random::~Random() {
  if (generator_) {
    delete generator_;
  }
  for (auto& el : udd_tf1_map_) {
    delete el.second;
  }
  for (auto& el : udd_tf2_map_) {
    delete el.second;
  }
  for (auto& el : udd_tf3_map_) {
    delete el.second;
  }
}

// -----------------------------------------------------------------------------
Random& Random::operator=(const Random& other) {
  if (&other != this) {
    if (generator_) {
      delete generator_;
    }
    generator_ = static_cast<TRandom*>(other.generator_->Clone());
  }
  return *this;
}

// -----------------------------------------------------------------------------
real_t Random::Uniform(real_t max) { return generator_->Uniform(max); }

// -----------------------------------------------------------------------------
real_t Random::Uniform(real_t min, real_t max) {
  return generator_->Uniform(min, max);
}

// -----------------------------------------------------------------------------
real_t Random::Gaus(real_t mean, real_t sigma) {
  return generator_->Gaus(mean, sigma);
}

// -----------------------------------------------------------------------------
real_t Random::Exp(real_t tau) { return generator_->Exp(tau); }

// -----------------------------------------------------------------------------
real_t Random::Landau(real_t mean, real_t sigma) {
  return generator_->Landau(mean, sigma);
}

// -----------------------------------------------------------------------------
real_t Random::PoissonD(real_t mean) { return generator_->PoissonD(mean); }

// -----------------------------------------------------------------------------
real_t Random::BreitWigner(real_t mean, real_t gamma) {
  return generator_->BreitWigner(mean, gamma);
}

// -----------------------------------------------------------------------------
unsigned Random::Integer(int max) { return generator_->Integer(max); }

// -----------------------------------------------------------------------------
int Random::Binomial(int ntot, real_t prob) {
  return generator_->Binomial(ntot, prob);
}

// -----------------------------------------------------------------------------
int Random::Poisson(real_t mean) { return generator_->Poisson(mean); }

// -----------------------------------------------------------------------------
MathArray<real_t, 2> Random::Circle(real_t r) {
  MathArray<double, 2> ret_double;
  generator_->Circle(ret_double[0], ret_double[1], static_cast<double>(r));
  return {static_cast<real_t>(ret_double[0]), static_cast<real_t>(ret_double[1])};
}

// -----------------------------------------------------------------------------
MathArray<real_t, 3> Random::Sphere(real_t r) {
  MathArray<double, 3> ret;
  generator_->Sphere(ret[0], ret[1], ret[2], r);
  return {static_cast<real_t>(ret[0]), static_cast<real_t>(ret[1]), static_cast<real_t>(ret[2])};
}

// -----------------------------------------------------------------------------
void Random::SetSeed(uint64_t seed) { generator_->SetSeed(seed); }

// -----------------------------------------------------------------------------
uint64_t Random::GetSeed() const { return generator_->GetSeed(); }

// -----------------------------------------------------------------------------
void Random::SetGenerator(TRandom* new_generator) {
  if (generator_) {
    delete generator_;
  }
  generator_ = new_generator;
}

// -----------------------------------------------------------------------------
template <typename TSample>
TSample DistributionRng<TSample>::Sample() {
  auto* rng = Simulation::GetActive()->GetRandom()->generator_;
  return SampleImpl(rng);
}
template real_t DistributionRng<real_t>::Sample();
template int DistributionRng<int>::Sample();

// -----------------------------------------------------------------------------
template <typename TSample>
MathArray<TSample, 2> DistributionRng<TSample>::Sample2() {
  auto* rng = Simulation::GetActive()->GetRandom()->generator_;
  return Sample2Impl(rng);
}
template MathArray<real_t, 2> DistributionRng<real_t>::Sample2();
template MathArray<int, 2> DistributionRng<int>::Sample2();

// -----------------------------------------------------------------------------
template <typename TSample>
MathArray<TSample, 2> DistributionRng<TSample>::Sample2Impl(TRandom* rng) {
  return MathArray<TSample, 2>({Sample(), Sample()});
}
template MathArray<real_t, 2> DistributionRng<real_t>::Sample2Impl(TRandom*);
template MathArray<int, 2> DistributionRng<int>::Sample2Impl(TRandom*);

// -----------------------------------------------------------------------------
template <typename TSample>
MathArray<TSample, 3> DistributionRng<TSample>::Sample3() {
  auto* rng = Simulation::GetActive()->GetRandom()->generator_;
  return Sample3Impl(rng);
}
template MathArray<real_t, 3> DistributionRng<real_t>::Sample3();
template MathArray<int, 3> DistributionRng<int>::Sample3();

// -----------------------------------------------------------------------------
template <typename TSample>
MathArray<TSample, 3> DistributionRng<TSample>::Sample3Impl(TRandom* rng) {
  return MathArray<TSample, 3>(
      {SampleImpl(rng), SampleImpl(rng), SampleImpl(rng)});
}
template MathArray<real_t, 3> DistributionRng<real_t>::Sample3Impl(TRandom*);
template MathArray<int, 3> DistributionRng<int>::Sample3Impl(TRandom*);

// -----------------------------------------------------------------------------
UniformRng::UniformRng(real_t min, real_t max) : min_(min), max_(max) {}
UniformRng::~UniformRng() {}
real_t UniformRng::SampleImpl(TRandom* rng) { return rng->Uniform(min_, max_); }

UniformRng Random::GetUniformRng(real_t min, real_t max) {
  return UniformRng(min, max);
}

// -----------------------------------------------------------------------------
GausRng::GausRng(real_t mean, real_t sigma) : mean_(mean), sigma_(sigma) {}
GausRng::~GausRng() {}
real_t GausRng::SampleImpl(TRandom* rng) { return rng->Gaus(mean_, sigma_); }

GausRng Random::GetGausRng(real_t mean, real_t sigma) {
  return GausRng(mean, sigma);
}

// -----------------------------------------------------------------------------
ExpRng::ExpRng(real_t tau) : tau_(tau) {}
ExpRng::~ExpRng() {}
real_t ExpRng::SampleImpl(TRandom* rng) { return rng->Exp(tau_); }

ExpRng Random::GetExpRng(real_t tau) { return ExpRng(tau); }

// -----------------------------------------------------------------------------
LandauRng::LandauRng(real_t mean, real_t sigma) : mean_(mean), sigma_(sigma) {}
LandauRng::~LandauRng() {}
real_t LandauRng::SampleImpl(TRandom* rng) {
  return rng->Landau(mean_, sigma_);
}

LandauRng Random::GetLandauRng(real_t mean, real_t sigma) {
  return LandauRng(mean, sigma);
}

// -----------------------------------------------------------------------------
PoissonDRng::PoissonDRng(real_t mean) : mean_(mean) {}
PoissonDRng::~PoissonDRng() {}
real_t PoissonDRng::SampleImpl(TRandom* rng) { return rng->PoissonD(mean_); }

PoissonDRng Random::GetPoissonDRng(real_t mean) { return PoissonDRng(mean); }

// -----------------------------------------------------------------------------
BreitWignerRng::BreitWignerRng(real_t mean, real_t gamma)
    : mean_(mean), gamma_(gamma) {}
BreitWignerRng::~BreitWignerRng() {}
real_t BreitWignerRng::SampleImpl(TRandom* rng) {
  return rng->BreitWigner(mean_, gamma_);
}

BreitWignerRng Random::GetBreitWignerRng(real_t mean, real_t gamma) {
  return BreitWignerRng(mean, gamma);
}

// -----------------------------------------------------------------------------
UserDefinedDistRng1D::UserDefinedDistRng1D(TF1* function, const char* option)
    : function_(function), option_(option) {}
UserDefinedDistRng1D::~UserDefinedDistRng1D() {}

// TODO(Lukas) after the update to ROOT 6.24 pass
// rng to GetRandom to avoid performance issue.
// also pass option_ to GetRandom.
real_t UserDefinedDistRng1D::SampleImpl(TRandom* rng) {
  auto min = function_->GetXmin();
  auto max = function_->GetXmax();
  return function_->GetRandom(min, max);
}
void UserDefinedDistRng1D::Draw(const char* option) { function_->Draw(option); }
TF1* UserDefinedDistRng1D::GetTF1() { return function_; }

// -----------------------------------------------------------------------------
UserDefinedDistRng1D Random::GetUserDefinedDistRng1D(
    double (*f)(const double*, const double*),
    const FixedSizeVector<real_t, 10>& params, real_t min, real_t max,
    const char* option) {
  TF1* tf1 = nullptr;
  UserDefinedDist udd{f, params, min, max};
  auto it = udd_tf1_map_.find(udd);
  if (it == udd_tf1_map_.end()) {
    tf1 = new TF1("", f, min, max, params.size());
    udd_tf1_map_[udd] = tf1;
    tf1->SetParameters(params[0], params[1], params[2], params[3], params[4],
                       params[5], params[6], params[7], params[8], params[9]);
  } else {
    tf1 = it->second;
  }
  return UserDefinedDistRng1D(tf1, option);
}

// -----------------------------------------------------------------------------
UserDefinedDistRng2D::UserDefinedDistRng2D(TF2* function, const char* option)
    : function_(function), option_(option) {}
UserDefinedDistRng2D::~UserDefinedDistRng2D() {}

// TODO(Lukas) after the update to ROOT 6.24 pass
// rng to GetRandom to avoid performance issue.
// also pass option_ to GetRandom.
real_t UserDefinedDistRng2D::SampleImpl(TRandom* rng) {
  auto min = function_->GetXmin();
  auto max = function_->GetXmax();
  return function_->GetRandom(min, max);
}
MathArray<real_t, 2> UserDefinedDistRng2D::Sample2Impl(TRandom* rng) {
  MathArray<double, 2> ret;
  function_->GetRandom2(ret[0], ret[1]);
  return {static_cast<real_t>(ret[0]), static_cast<real_t>(ret[1])};
}
void UserDefinedDistRng2D::Draw(const char* option) { function_->Draw(option); }
TF2* UserDefinedDistRng2D::GetTF2() { return function_; }

UserDefinedDistRng2D Random::GetUserDefinedDistRng2D(
    double (*f)(const double*, const double*),
    const FixedSizeVector<real_t, 10>& params, real_t xmin, real_t xmax,
    real_t ymin, real_t ymax, const char* option) {
  TF2* tf2 = nullptr;
  UserDefinedDist udd{f, params, xmin, xmax, ymin, ymax};
  auto it = udd_tf2_map_.find(udd);
  if (it == udd_tf2_map_.end()) {
    tf2 = new TF2("", f, xmin, xmax, ymin, ymax, params.size());
    udd_tf2_map_[udd] = tf2;
    tf2->SetParameters(params[0], params[1], params[2], params[3], params[4],
                       params[5], params[6], params[7], params[8], params[9]);
  } else {
    tf2 = it->second;
  }
  return UserDefinedDistRng2D(tf2, option);
}

// -----------------------------------------------------------------------------
UserDefinedDistRng3D::UserDefinedDistRng3D(TF3* function, const char* option)
    : function_(function), option_(option) {}
UserDefinedDistRng3D::~UserDefinedDistRng3D() {}

// TODO(Lukas) after the update to ROOT 6.24 pass
// rng to GetRandom to avoid performance issue.
// also pass option_ to GetRandom.
real_t UserDefinedDistRng3D::SampleImpl(TRandom* rng) {
  auto min = function_->GetXmin();
  auto max = function_->GetXmax();
  return function_->GetRandom(min, max);
}
MathArray<real_t, 2> UserDefinedDistRng3D::Sample2Impl(TRandom* rng) {
  MathArray<double, 2> ret;
  function_->GetRandom2(ret[0], ret[1]);
  return {static_cast<real_t>(ret[0]), static_cast<real_t>(ret[1])};
}
MathArray<real_t, 3> UserDefinedDistRng3D::Sample3Impl(TRandom* rng) {
  MathArray<double, 3> ret;
  function_->GetRandom3(ret[0], ret[1], ret[2]);
  return {static_cast<real_t>(ret[0]), static_cast<real_t>(ret[1]), static_cast<real_t>(ret[2])};
}
void UserDefinedDistRng3D::Draw(const char* option) { function_->Draw(option); }
TF3* UserDefinedDistRng3D::GetTF3() { return function_; }

UserDefinedDistRng3D Random::GetUserDefinedDistRng3D(
    double (*f)(const double*, const double*),
    const FixedSizeVector<real_t, 10>& params, real_t xmin, real_t xmax,
    real_t ymin, real_t ymax, real_t zmin, real_t zmax, const char* option) {
  TF3* tf3 = nullptr;
  UserDefinedDist udd{f, params, xmin, xmax, ymin, ymax, zmin, zmax};
  auto it = udd_tf3_map_.find(udd);
  if (it == udd_tf3_map_.end()) {
    tf3 = new TF3("", f, xmin, xmax, ymin, ymax, zmin, zmax, params.size());
    udd_tf3_map_[udd] = tf3;
    tf3->SetParameters(params[0], params[1], params[2], params[3], params[4],
                       params[5], params[6], params[7], params[8], params[9]);
  } else {
    tf3 = it->second;
  }
  return UserDefinedDistRng3D(tf3, option);
}

// -----------------------------------------------------------------------------
BinomialRng::BinomialRng(int ntot, real_t prob) : ntot_(ntot), prob_(prob) {}
BinomialRng::~BinomialRng() {}
int BinomialRng::SampleImpl(TRandom* rng) {
  return rng->Binomial(ntot_, prob_);
}

BinomialRng Random::GetBinomialRng(int ntot, real_t prob) {
  return BinomialRng(ntot, prob);
}

// -----------------------------------------------------------------------------
PoissonRng::PoissonRng(real_t mean) : mean_(mean) {}
PoissonRng::~PoissonRng() {}
int PoissonRng::SampleImpl(TRandom* rng) { return rng->Poisson(mean_); }

PoissonRng Random::GetPoissonRng(real_t mean) { return PoissonRng(mean); }

}  // namespace bdm
