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

#include "core/util/random.h"
#include <TF1.h>
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
double Random::Uniform(double max) { return generator_->Uniform(max); }

// -----------------------------------------------------------------------------
double Random::Uniform(double min, double max) {
  return generator_->Uniform(min, max);
}

// -----------------------------------------------------------------------------
double Random::Gaus(double mean, double sigma) {
  return generator_->Gaus(mean, sigma);
}

// -----------------------------------------------------------------------------
double Random::Exp(double tau) { return generator_->Exp(tau); }

// -----------------------------------------------------------------------------
double Random::Landau(double mean, double sigma) {
  return generator_->Landau(mean, sigma);
}

// -----------------------------------------------------------------------------
double Random::PoissonD(double mean) { return generator_->PoissonD(mean); }

// -----------------------------------------------------------------------------
double Random::BreitWigner(double mean, double gamma) {
  return generator_->BreitWigner(mean, gamma);
}

// -----------------------------------------------------------------------------
unsigned Random::Integer(int max) { return generator_->Integer(max); }

// -----------------------------------------------------------------------------
int Random::Binomial(int ntot, double prob) {
  return generator_->Binomial(ntot, prob);
}

// -----------------------------------------------------------------------------
int Random::Poisson(double mean) { return generator_->Poisson(mean); }

// -----------------------------------------------------------------------------
MathArray<double, 2> Random::Circle(double r) {
  MathArray<double, 2> ret;
  generator_->Circle(ret[0], ret[1], r);
  return ret;
}

// -----------------------------------------------------------------------------
MathArray<double, 3> Random::Sphere(double r) {
  MathArray<double, 3> ret;
  generator_->Sphere(ret[0], ret[1], ret[2], r);
  return ret;
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
template double DistributionRng<double>::Sample();
template int DistributionRng<int>::Sample();

// -----------------------------------------------------------------------------
UniformRng::UniformRng(double min, double max) : min_(min), max_(max) {}
UniformRng::~UniformRng() {}
double UniformRng::SampleImpl(TRandom* rng) { return rng->Uniform(min_, max_); }

UniformRng Random::GetUniformRng(double min, double max) {
  return UniformRng(min, max);
}

// -----------------------------------------------------------------------------
GausRng::GausRng(double mean, double sigma) : mean_(mean), sigma_(sigma) {}
GausRng::~GausRng() {}
double GausRng::SampleImpl(TRandom* rng) { return rng->Gaus(mean_, sigma_); }

GausRng Random::GetGausRng(double mean, double sigma) {
  return GausRng(mean, sigma);
}

// -----------------------------------------------------------------------------
ExpRng::ExpRng(double tau) : tau_(tau) {}
ExpRng::~ExpRng() {}
double ExpRng::SampleImpl(TRandom* rng) { return rng->Exp(tau_); }

ExpRng Random::GetExpRng(double tau) { return ExpRng(tau); }

// -----------------------------------------------------------------------------
LandauRng::LandauRng(double mean, double sigma) : mean_(mean), sigma_(sigma) {}
LandauRng::~LandauRng() {}
double LandauRng::SampleImpl(TRandom* rng) {
  return rng->Landau(mean_, sigma_);
}

LandauRng Random::GetLandauRng(double mean, double sigma) {
  return LandauRng(mean, sigma);
}

// -----------------------------------------------------------------------------
PoissonDRng::PoissonDRng(double mean) : mean_(mean) {}
PoissonDRng::~PoissonDRng() {}
double PoissonDRng::SampleImpl(TRandom* rng) { return rng->PoissonD(mean_); }

PoissonDRng Random::GetPoissonDRng(double mean) { return PoissonDRng(mean); }

// -----------------------------------------------------------------------------
BreitWignerRng::BreitWignerRng(double mean, double gamma)
    : mean_(mean), gamma_(gamma) {}
BreitWignerRng::~BreitWignerRng() {}
double BreitWignerRng::SampleImpl(TRandom* rng) {
  return rng->BreitWigner(mean_, gamma_);
}

BreitWignerRng Random::GetBreitWignerRng(double mean, double gamma) {
  return BreitWignerRng(mean, gamma);
}

// -----------------------------------------------------------------------------
UserDefinedDistRng::UserDefinedDistRng(TF1* function) : function_(function) {}
UserDefinedDistRng::~UserDefinedDistRng() {}

// TODO(Lukas) after the update to ROOT 6.24 pass
// rng to GetRandom to avoid performance issue.
double UserDefinedDistRng::SampleImpl(TRandom* rng) {
  auto min = function_->GetXmin();
  auto max = function_->GetXmax();
  return function_->GetRandom(min, max);
}
void UserDefinedDistRng::Draw() { function_->Draw(); }

UserDefinedDistRng Random::GetUserDefinedDistRng(
    double (*f)(const double*, const double*),
    const FixedSizeVector<double, 10>& params, double min, double max) {
  TF1* tf1 = nullptr;
  UserDefinedDist udd{f, params, min, max};
  auto it = udd_tf1_map_.find(udd);
  if (it == udd_tf1_map_.end()) {
    tf1 = new TF1("UserDefinedDistRng", f, min, max, params.size());
    udd_tf1_map_[udd] = tf1;
    tf1->SetParameters(params[0], params[1], params[2], params[3], params[3],
                       params[4], params[5], params[6], params[7], params[8],
                       params[9]);
  } else {
    tf1 = it->second;
  }
  return UserDefinedDistRng(tf1);
}

// -----------------------------------------------------------------------------
BinomialRng::BinomialRng(int ntot, double prob) : ntot_(ntot), prob_(prob) {}
BinomialRng::~BinomialRng() {}
int BinomialRng::SampleImpl(TRandom* rng) {
  return rng->Binomial(ntot_, prob_);
}

BinomialRng Random::GetBinomialRng(int ntot, double prob) {
  return BinomialRng(ntot, prob);
}

// -----------------------------------------------------------------------------
PoissonRng::PoissonRng(double mean) : mean_(mean) {}
PoissonRng::~PoissonRng() {}
int PoissonRng::SampleImpl(TRandom* rng) { return rng->Poisson(mean_); }

PoissonRng Random::GetPoissonRng(double mean) { return PoissonRng(mean); }

}  // namespace bdm
