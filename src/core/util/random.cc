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
#include <cmath>
#include <random>
#include "core/simulation.h"


namespace bdm {

// -----------------------------------------------------------------------------
// Seed mt_engine_ from a non-deterministic source so each instance starts
// with a unique sequence by default, matching TRandom3's time-based seeding.
Random::Random()
    : mt_engine_(std::random_device{}()), generator_(new TRandom3()) {}


// -----------------------------------------------------------------------------
Random::Random(TRootIOCtor*) {}

// -----------------------------------------------------------------------------
// In random.cc — the missing definition
std::mt19937_64& Random::GetEngine() {
  return mt_engine_;
}

// -----------------------------------------------------------------------------
Random::Random(const Random& other)
    : mt_engine_(other.mt_engine_),
      generator_(static_cast<TRandom*>(other.generator_->Clone())) {}


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
    mt_engine_ = other.mt_engine_;
    if (generator_) {
      delete generator_;
    }
    generator_ = static_cast<TRandom*>(other.generator_->Clone());
  }
  return *this;
}

// -----------------------------------------------------------------------------
real_t Random::Uniform(real_t max) {
  std::uniform_real_distribution<real_t> dist(static_cast<real_t>(0), max);
  return dist(mt_engine_);
}

// -----------------------------------------------------------------------------
real_t Random::Uniform(real_t min, real_t max) {
  std::uniform_real_distribution<real_t> dist(min, max);
  return dist(mt_engine_);
}

// -----------------------------------------------------------------------------
real_t Random::Gaus(real_t mean, real_t sigma) {
  std::normal_distribution<real_t> dist(mean, sigma);
  return dist(mt_engine_);
}


// -----------------------------------------------------------------------------
// Uses std::exponential_distribution driven by mt_engine_.
// ROOT's tau is the mean/scale parameter, while
// std::exponential_distribution expects the rate parameter lambda = 1 / tau.
real_t Random::Exp(real_t tau) {
  std::exponential_distribution<real_t> dist(static_cast<real_t>(1.0) / tau);
  return dist(mt_engine_);
}

// -----------------------------------------------------------------------------
real_t Random::Landau(real_t mean, real_t sigma) {
  return generator_->Landau(mean, sigma);
}

// -----------------------------------------------------------------------------
// Uses std::poisson_distribution driven by mt_engine_.
// Note: ROOT's PoissonD may switch to a Gaussian approximation for very large
// means, while std::poisson_distribution always samples from the true Poisson
// distribution.  For the mean values typical of BioDynaMo simulations the two
// are statistically equivalent.  The std distribution returns an integer type
// (long); the result is cast back to real_t to preserve the original API.
real_t Random::PoissonD(real_t mean) {
  std::poisson_distribution<long> dist(static_cast<double>(mean));
  return static_cast<real_t>(dist(mt_engine_));
}

// -----------------------------------------------------------------------------
real_t Random::BreitWigner(real_t mean, real_t gamma) {
  return generator_->BreitWigner(mean, gamma);
}

// -----------------------------------------------------------------------------
// Uses std::uniform_int_distribution driven by mt_engine_.
// ROOT's Integer(max) returns values in [0, max - 1]; std uses inclusive
// bounds, so the upper bound is max - 1.
unsigned Random::Integer(int max) {
  std::uniform_int_distribution<unsigned> dist(
      0u, static_cast<unsigned>(max) - 1u);
  return dist(mt_engine_);
}


// -----------------------------------------------------------------------------
// Uses std::binomial_distribution driven by mt_engine_.
// Parameters map directly: ntot = number of trials, prob = success probability.
int Random::Binomial(int ntot, real_t prob) {
  std::binomial_distribution<int> dist(ntot, static_cast<double>(prob));
  return dist(mt_engine_);
}

// -----------------------------------------------------------------------------
// Uses std::poisson_distribution driven by mt_engine_.
// ROOT's Poisson(mean) and std::poisson_distribution both take the mean
// directly, so no parameter transformation is required.
int Random::Poisson(real_t mean) {
  std::poisson_distribution<int> dist(static_cast<double>(mean));
  return dist(mt_engine_);
}

// -----------------------------------------------------------------------------
// Generates a point uniformly distributed on the circumference of a circle
// with radius r by sampling theta ~ Uniform(0, 2*pi) and returning
// (r*cos(theta), r*sin(theta)).  Pi is defined locally because M_PI is not
// portable across platforms / compilers.
MathArray<real_t, 2> Random::Circle(real_t r) {
  constexpr real_t kPi =
      static_cast<real_t>(3.141592653589793238462643383279502884);
  constexpr real_t kTwoPi = static_cast<real_t>(2) * kPi;

  std::uniform_real_distribution<real_t> dist(static_cast<real_t>(0), kTwoPi);
  const real_t theta = dist(mt_engine_);

  return {r * std::cos(theta), r * std::sin(theta)};
}

// -----------------------------------------------------------------------------
// Generates a point uniformly distributed on the surface of a sphere with
// radius r using the standard Gaussian-vector normalisation method:
// a 3D vector with i.i.d. standard normal components is rotationally
// symmetric, so normalising it produces a direction uniformly distributed
// on the sphere.  Scaling by r places the point on the requested surface.
// Resample in the (statistically vanishing) case norm == 0 to avoid 0/0.
MathArray<real_t, 3> Random::Sphere(real_t r) {
  std::normal_distribution<real_t> dist(static_cast<real_t>(0),
                                        static_cast<real_t>(1));

  real_t x = 0;
  real_t y = 0;
  real_t z = 0;
  real_t norm = 0;

  do {
    x = dist(mt_engine_);
    y = dist(mt_engine_);
    z = dist(mt_engine_);
    norm = std::sqrt(x * x + y * y + z * z);
  } while (norm == static_cast<real_t>(0));

  const real_t scale = r / norm;
  return {x * scale, y * scale, z * scale};
}

// -----------------------------------------------------------------------------
void Random::SetSeed(uint64_t seed) {
  last_seed_ = seed;
  mt_engine_.seed(seed);
  generator_->SetSeed(seed);
}

// -----------------------------------------------------------------------------
uint64_t Random::GetSeed() const { return last_seed_; }

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
UniformRng::~UniformRng() = default;
// Uses std::uniform_real_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
real_t UniformRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::uniform_real_distribution<real_t> dist(min_, max_);
  return dist(engine);
}
UniformRng Random::GetUniformRng(real_t min, real_t max) const {
  return UniformRng(min, max);
}

// -----------------------------------------------------------------------------
GausRng::GausRng(real_t mean, real_t sigma) : mean_(mean), sigma_(sigma) {}
GausRng::~GausRng() = default;
// Uses std::normal_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
real_t GausRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::normal_distribution<real_t> dist(mean_, sigma_);
  return dist(engine);
}

GausRng Random::GetGausRng(real_t mean, real_t sigma) const {
  return GausRng(mean, sigma);
}

// -----------------------------------------------------------------------------
ExpRng::ExpRng(real_t tau) : tau_(tau) {}
ExpRng::~ExpRng() = default;
// Uses std::exponential_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
real_t ExpRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::exponential_distribution<real_t> dist(static_cast<real_t>(1.0) / tau_);
  return dist(engine);
}

ExpRng Random::GetExpRng(real_t tau) const { return ExpRng(tau); }

// -----------------------------------------------------------------------------
LandauRng::LandauRng(real_t mean, real_t sigma) : mean_(mean), sigma_(sigma) {}
LandauRng::~LandauRng() = default;
real_t LandauRng::SampleImpl(TRandom* rng) {
  return rng->Landau(mean_, sigma_);
}

LandauRng Random::GetLandauRng(real_t mean, real_t sigma) const {
  return LandauRng(mean, sigma);
}

// -----------------------------------------------------------------------------
PoissonDRng::PoissonDRng(real_t mean) : mean_(mean) {}
PoissonDRng::~PoissonDRng() = default;
// Uses std::poisson_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
// See Random::PoissonD for notes on the large-mean Gaussian approximation
// used by ROOT.
real_t PoissonDRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::poisson_distribution<long> dist(static_cast<double>(mean_));
  return static_cast<real_t>(dist(engine));
}

PoissonDRng Random::GetPoissonDRng(real_t mean) const {
  return PoissonDRng(mean);
}

// -----------------------------------------------------------------------------
BreitWignerRng::BreitWignerRng(real_t mean, real_t gamma)
    : mean_(mean), gamma_(gamma) {}
BreitWignerRng::~BreitWignerRng() = default;
real_t BreitWignerRng::SampleImpl(TRandom* rng) {
  return rng->BreitWigner(mean_, gamma_);
}

BreitWignerRng Random::GetBreitWignerRng(real_t mean, real_t gamma) const {
  return BreitWignerRng(mean, gamma);
}

// -----------------------------------------------------------------------------
UserDefinedDistRng1D::UserDefinedDistRng1D(TF1* function, const char* option)
    : function_(function), option_(option) {}
UserDefinedDistRng1D::~UserDefinedDistRng1D() = default;

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
UserDefinedDistRng2D::~UserDefinedDistRng2D() = default;

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
UserDefinedDistRng3D::~UserDefinedDistRng3D() = default;

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
  return {static_cast<real_t>(ret[0]), static_cast<real_t>(ret[1]),
          static_cast<real_t>(ret[2])};
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
BinomialRng::~BinomialRng() = default;
// Uses std::binomial_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
int BinomialRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::binomial_distribution<int> dist(ntot_, static_cast<double>(prob_));
  return dist(engine);
}

BinomialRng Random::GetBinomialRng(int ntot, real_t prob) const {
  return BinomialRng(ntot, prob);
}

// -----------------------------------------------------------------------------
PoissonRng::PoissonRng(real_t mean) : mean_(mean) {}
PoissonRng::~PoissonRng() = default;
// Uses std::poisson_distribution driven by Random::GetEngine().
// The TRandom* parameter is ignored; it exists only to satisfy the virtual
// interface, which will be updated in a future refactor step.
int PoissonRng::SampleImpl(TRandom* /*rng*/) {
  auto& engine = Simulation::GetActive()->GetRandom()->GetEngine();
  std::poisson_distribution<int> dist(static_cast<double>(mean_));
  return dist(engine);
}

PoissonRng Random::GetPoissonRng(real_t mean) const { return PoissonRng(mean); }

}  // namespace bdm
