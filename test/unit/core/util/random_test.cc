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
#include <Math/DistFunc.h>
#include <TF1.h>
#include <TRandom3.h>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <numeric>
#include <vector>
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_util.h"

namespace bdm {

// The engine is now std::mt19937_64 so we no longer compare sample-by-sample
// against TRandom3.  Instead we verify three properties that must hold
// regardless of the underlying engine:
//   1. Reproducibility  – the same seed always produces the same sequence.
//   2. Range            – every value falls in the requested interval.
//   3. RNG-object parity – GetUniformRng().Sample() and Uniform(min,max)
//                          draw from the same engine and stay in range.
  TEST(RandomTest, Uniform) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
 
  // --- 1. Reproducibility: same seed → identical sequence ---------------
  
  random->SetSeed(42);
  std::vector<real_t> run1;

  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Uniform());
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(run1[i], random->Uniform());
  }
  // --- 2a. Range check: Uniform(max) must be in [0, max) ----------------
  random->SetSeed(42);
  for (uint64_t i = 1; i <= 10; i++) {
    const real_t max = static_cast<real_t>(i);
    const real_t val = random->Uniform(max);
    EXPECT_GE(val, static_cast<real_t>(0));
    EXPECT_LT(val, max);
  }

  // --- 2b. Range check: Uniform(min, max) must be in [min, max) ---------
  random->SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    const real_t lo = static_cast<real_t>(i);
    const real_t hi = lo + static_cast<real_t>(2);
    const real_t val = random->Uniform(lo, hi);
    EXPECT_GE(val, lo);
    EXPECT_LT(val, hi);
  }
  // --- 3. GetUniformRng: samples must stay within [3, 4) ----------------
  auto distrng = random->GetUniformRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    const real_t val = distrng.Sample();
    EXPECT_GE(val, static_cast<real_t>(3));
    EXPECT_LT(val, static_cast<real_t>(4));

  }
}
// UniformArray is a thin wrapper around Uniform(); the key properties to
// verify are reproducibility and per-element range correctness.
TEST(RandomTest, UniformArray) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  // --- 1. Reproducibility: same seed → identical array -----------------
  random->SetSeed(42);
  auto run1 = random->UniformArray<5>();
  random->SetSeed(42);
  auto run2 = random->UniformArray<5>();
  for (uint64_t i = 0; i < 5; i++) {
    EXPECT_REAL_EQ(run1[i], run2[i]);
  }
  // --- 2a. UniformArray<N>(): all elements in [0, 1) -------------------
  random->SetSeed(42);
  auto result = random->UniformArray<5>();
  for (uint64_t i = 0; i < 5; i++) {
    EXPECT_GE(result[i], static_cast<real_t>(0));
    EXPECT_LT(result[i], static_cast<real_t>(1));
  }

   // --- 2b. UniformArray<N>(max): all elements in [0, max) --------------
  random->SetSeed(42);
  auto result1 = random->UniformArray<2>(static_cast<real_t>(8.3));
  for (uint64_t i = 0; i < 2; i++) {
    EXPECT_GE(result1[i], static_cast<real_t>(0));
    EXPECT_LT(result1[i], static_cast<real_t>(8.3));
  }

  // --- 2c. UniformArray<N>(min, max): all elements in [min, max) -------
  random->SetSeed(42);
  auto result2 = random->UniformArray<12>(static_cast<real_t>(5.1),
                                          static_cast<real_t>(9.87));

  for (uint64_t i = 0; i < 12; i++) {
    EXPECT_GE(result2[i], static_cast<real_t>(5.1));
    EXPECT_LT(result2[i], static_cast<real_t>(9.87));
  }
}
// Gaus now uses std::normal_distribution so per-sample parity with TRandom3
// no longer holds.  We verify:
//   1. Reproducibility  – same seed → same sequence.
//   2. Statistical      – with N=10000 samples the empirical mean and standard
//                         deviation must land within a tight tolerance of the
//                         requested parameters (CLT guarantees this).
//   3. GausRng object   – GetGausRng() draws from the same engine and its
//                         statistics match the requested parameters.
TEST(RandomTest, Gaus) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<real_t> run1;

  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Gaus());
  }

  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(run1[i], random->Gaus());
  }

  // --- 2. Statistical check for Gaus(mean, sigma) -----------------------
  // With 10 000 samples the standard error of the mean is sigma/sqrt(N)
  // ≈ 2/100 = 0.02, so a tolerance of 0.1 is very conservative.
  const uint64_t kN = 10000;
  const real_t kMean = static_cast<real_t>(5);
  const real_t kSigma = static_cast<real_t>(2);

  random->SetSeed(123);
  real_t sum = 0, sum_sq = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = random->Gaus(kMean, kSigma);
    sum += v;
    sum_sq += v * v;
  }
  const real_t sample_mean = sum / static_cast<real_t>(kN);
  const real_t sample_var =
      sum_sq / static_cast<real_t>(kN) - sample_mean * sample_mean;
  EXPECT_NEAR(static_cast<double>(sample_mean), static_cast<double>(kMean),
              0.1);
  EXPECT_NEAR(static_cast<double>(std::sqrt(sample_var)),
              static_cast<double>(kSigma), 0.1);

  // --- 3. GetGausRng statistical check ----------------------------------
  auto distrng = random->GetGausRng(static_cast<real_t>(3),
                                    static_cast<real_t>(4));
  sum = 0;
  sum_sq = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = distrng.Sample();
    sum += v;
    sum_sq += v * v;
  }
  const real_t rng_mean = sum / static_cast<real_t>(kN);
  const real_t rng_var =
      sum_sq / static_cast<real_t>(kN) - rng_mean * rng_mean;
  EXPECT_NEAR(static_cast<double>(rng_mean), 3.0, 0.1);
  EXPECT_NEAR(static_cast<double>(std::sqrt(rng_var)), 4.0, 0.2);

  }

  // Exp now uses std::exponential_distribution so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence.
//   2. Range           – every sample is non-negative.
//   3. Statistical     – with N=10000 samples the empirical mean is close to
//                        the requested tau (CLT).
//   4. ExpRng object   – GetExpRng() draws from the same engine, stays in
//                        range and matches the requested mean.
TEST(RandomTest, Exp) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<real_t> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Exp(static_cast<real_t>(5)));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(run1[i], random->Exp(static_cast<real_t>(5)));
  }

    // --- 2 & 3. Range + statistical check for Exp(tau) --------------------
  const uint64_t kN = 10000;
  const real_t kTau = static_cast<real_t>(5);
  random->SetSeed(123);
  real_t sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = random->Exp(kTau);
    EXPECT_GE(v, static_cast<real_t>(0));
    sum += v;
  }
  const real_t sample_mean = sum / static_cast<real_t>(kN);
  EXPECT_NEAR(static_cast<double>(sample_mean), static_cast<double>(kTau),
              0.2);

  // --- 4. GetExpRng range + statistical check ---------------------------
  auto distrng = random->GetExpRng(kTau);
  sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = distrng.Sample();
    EXPECT_GE(v, static_cast<real_t>(0));
    sum += v;
  }
  const real_t rng_mean = sum / static_cast<real_t>(kN);
  EXPECT_NEAR(static_cast<double>(rng_mean), static_cast<double>(kTau), 0.2);
}

TEST(RandomTest, Landau) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.Landau()), random->Landau());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.Landau(i)), random->Landau(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.Landau(i, i + 2)),
                   random->Landau(i, i + 2));
  }

  auto distrng = random->GetLandauRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.Landau(3, 4)),
                   distrng.Sample());
  }
}

// PoissonD now uses std::poisson_distribution so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence.
//   2. Range           – every sample is non-negative.
//   3. Statistical     – with N=10000 samples the empirical mean and variance
//                        are close to the requested mean.
//   4. PoissonDRng     – GetPoissonDRng() draws from the same engine and its
//                        statistics match the requested mean.
TEST(RandomTest, PoissonD) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;
  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<real_t> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->PoissonD(static_cast<real_t>(8)));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(run1[i], random->PoissonD(static_cast<real_t>(8)));
  }

  // --- 2 & 3. Range + statistical check for PoissonD(mean) --------------
  const uint64_t kN = 10000;
  const real_t kMean = static_cast<real_t>(8);
  random->SetSeed(123);
  real_t sum = 0, sum_sq = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = random->PoissonD(kMean);
    EXPECT_GE(v, static_cast<real_t>(0));
    sum += v;
    sum_sq += v * v;
  }
  const real_t sample_mean = sum / static_cast<real_t>(kN);
  const real_t sample_var =
      sum_sq / static_cast<real_t>(kN) - sample_mean * sample_mean;
  EXPECT_NEAR(static_cast<double>(sample_mean), static_cast<double>(kMean),
              0.2);
  EXPECT_NEAR(static_cast<double>(sample_var), static_cast<double>(kMean),
              0.5);

  // --- 4. GetPoissonDRng range + statistical check ----------------------
  auto distrng = random->GetPoissonDRng(kMean);
  sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const real_t v = distrng.Sample();
    EXPECT_GE(v, static_cast<real_t>(0));
    sum += v;
  }
  const real_t rng_mean = sum / static_cast<real_t>(kN);
  EXPECT_NEAR(static_cast<double>(rng_mean), static_cast<double>(kMean), 0.2);
}

TEST(RandomTest, BreitWigner) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.BreitWigner()),
                   random->BreitWigner());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.BreitWigner(i)),
                   random->BreitWigner(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.BreitWigner(i, i + 2)),
                   random->BreitWigner(i, i + 2));
  }

  auto distrng = random->GetBreitWignerRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(static_cast<real_t>(reference.BreitWigner(3, 4)),
                   distrng.Sample());
  }
}

TEST(RandomTest, UserDefinedDistRng1D) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  auto function = [](const double* x, const double* params) {
    return ROOT::Math::lognormal_pdf(*x, params[0], params[1]);
  };
  real_t min = 1;
  real_t max = 4;
  TF1 reference("", function, min, max, 2);
  reference.SetParameters(1.1, 1.2);

  // workaround until update to 6.24
  gRandom->SetSeed(42);
  std::vector<real_t> expected;
  for (uint64_t i = 0; i < 10; i++) {
    expected.push_back(reference.GetRandom(min, max));
  }

  gRandom->SetSeed(42);
  auto distrng =
      random->GetUserDefinedDistRng1D(function, {1.1, 1.2}, min, max);
  std::vector<real_t> actual;
  for (uint64_t i = 0; i < 10; i++) {
    actual.push_back(distrng.Sample());
  }
  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_NEAR(expected[i], actual[i], abs_error<real_t>::value);
  }
}

// This test is neither a proper death test nor a proper function test. We run
// this test, to see if it is possible to initialize the UserDefinedDistRng1D
// in a parallel region because the generator is based on ROOT's TF1 class which
// seems to have trouble when the constructor is called in parallel. Before the
// fix provided in PR #208, this test would fail roughly 2/3 times.
TEST(RandomTest, UserDefinedDistRng1DParallel) {
  Simulation simulation(TEST_NAME);
  std::vector<real_t> results;
#pragma omp parallel shared(simulation, results)
  {
    auto* random = simulation.GetRandom();
    auto function = [](const double* x, const double* params) {
      return ROOT::Math::lognormal_pdf(*x, params[0], params[1]);
    };
    real_t min = 1;
    real_t max = 4;
    size_t n_samples = 10000;
    real_t result = 0.0;
    auto distrng =
        random->GetUserDefinedDistRng1D(function, {1.1, 1.2}, min, max);
    for (size_t i = 0; i < n_samples; i++) {
      result += distrng.Sample();
    }
#pragma omp critical
    { results.push_back(result / n_samples); }
  }
  real_t sum = std::accumulate(results.begin(), results.end(), 0.0);
  EXPECT_LT(sum, std::numeric_limits<real_t>::max());
}

// This test is neither a proper death test nor a proper function test. We run
// this test, to see if it is possible to initialize the UserDefinedDistRng2D
// in a parallel region because the generator is based on ROOT's TF2 class which
// seems to have trouble when the constructor is called in parallel. Before the
// fix provided in PR #208, this test would fail roughly 2/3 times.
TEST(RandomTest, UserDefinedDistRng2DParallel) {
  Simulation simulation(TEST_NAME);
  std::vector<real_t> results;
#pragma omp parallel shared(simulation, results)
  {
    auto* random = simulation.GetRandom();
    auto function = [](const double* x, const double* params) {
      return ROOT::Math::lognormal_pdf(*x, params[0], params[1]);
    };
    real_t min = 1;
    real_t max = 4;
    size_t n_samples = 10000;
    real_t result = 0.0;
    auto distrng = random->GetUserDefinedDistRng2D(function, {1.1, 1.2}, min,
                                                   max, min, max);
    for (size_t i = 0; i < n_samples; i++) {
      result += distrng.Sample2().Norm();
    }
#pragma omp critical
    { results.push_back(result / n_samples); }
  }
  real_t sum = std::accumulate(results.begin(), results.end(), 0.0);
  EXPECT_LT(sum, std::numeric_limits<real_t>::max());
}

// This test is neither a proper death test nor a proper function test. We run
// this test, to see if it is possible to initialize the UserDefinedDistRng3D
// in a parallel region because the generator is based on ROOT's TF3 class which
// seems to have trouble when the constructor is called in parallel. Before the
// fix provided in PR #208, this test would fail roughly 2/3 times.
TEST(RandomTest, UserDefinedDistRng3DParallel) {
  Simulation simulation(TEST_NAME);
  std::vector<real_t> results;
#pragma omp parallel shared(simulation, results)
  {
    auto* random = simulation.GetRandom();
    auto function = [](const double* x, const double* params) {
      return ROOT::Math::lognormal_pdf(*x, params[0], params[1]);
    };
    real_t min = 1;
    real_t max = 4;
    size_t n_samples = 10000;
    real_t result = 0.0;
    auto distrng = random->GetUserDefinedDistRng3D(function, {1.1, 1.2}, min,
                                                   max, min, max, min, max);
    for (size_t i = 0; i < n_samples; i++) {
      result += distrng.Sample3().Norm();
    }
#pragma omp critical
    { results.push_back(result / n_samples); }
  }
  real_t sum = std::accumulate(results.begin(), results.end(), 0.0);
  EXPECT_LT(sum, std::numeric_limits<real_t>::max());
}

// Binomial now uses std::binomial_distribution so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence.
//   2. Range           – every sample is in [0, ntot].
//   3. Statistical     – with N=10000 samples the empirical mean is close to
//                        ntot * prob.
//   4. BinomialRng     – GetBinomialRng() draws from the same engine, stays
//                        in range and matches the requested mean.
TEST(RandomTest, Binomial) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  const int kNtot = 20;
  const real_t kProb = static_cast<real_t>(0.4);
  const real_t kExpectedMean =
      static_cast<real_t>(kNtot) * kProb;

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<int> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Binomial(kNtot, kProb));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(run1[i], random->Binomial(kNtot, kProb));
  }

  // --- 2 & 3. Range + statistical check for Binomial(ntot, prob) --------
  const uint64_t kN = 10000;
  random->SetSeed(123);
  int64_t sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const int v = random->Binomial(kNtot, kProb);
    EXPECT_GE(v, 0);
    EXPECT_LE(v, kNtot);
    sum += v;
  }
  const double sample_mean =
      static_cast<double>(sum) / static_cast<double>(kN);
  EXPECT_NEAR(sample_mean, static_cast<double>(kExpectedMean), 0.2);

  // --- 4. GetBinomialRng range + statistical check ----------------------
  auto distrng = random->GetBinomialRng(kNtot, kProb);
  sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const int v = distrng.Sample();
    EXPECT_GE(v, 0);
    EXPECT_LE(v, kNtot);
    sum += v;
  }
  const double rng_mean =
      static_cast<double>(sum) / static_cast<double>(kN);
  EXPECT_NEAR(rng_mean, static_cast<double>(kExpectedMean), 0.2);
}

// Poisson now uses std::poisson_distribution so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence.
//   2. Support         – every sample is non-negative.
//   3. Statistical     – with N=10000 samples the empirical mean and variance
//                        are close to the requested mean.
//   4. PoissonRng      – GetPoissonRng() draws from the same engine and its
//                        empirical mean matches the requested mean.
TEST(RandomTest, Poisson) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<int> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Poisson(static_cast<real_t>(8)));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(run1[i], random->Poisson(static_cast<real_t>(8)));
  }

  // --- 2 & 3. Support + statistical check for Poisson(mean) -------------
  const uint64_t kN = 10000;
  const real_t kMean = static_cast<real_t>(8);
  random->SetSeed(123);
  double sum = 0;
  double sum_sq = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const int v = random->Poisson(kMean);
    EXPECT_GE(v, 0);
    sum += v;
    sum_sq += static_cast<double>(v) * v;
  }
  const double sample_mean = sum / static_cast<double>(kN);
  const double sample_var =
      sum_sq / static_cast<double>(kN) - sample_mean * sample_mean;
  EXPECT_NEAR(sample_mean, static_cast<double>(kMean), 0.2);
  EXPECT_NEAR(sample_var, static_cast<double>(kMean), 0.5);

  // --- 4. GetPoissonRng range + statistical check -----------------------
  auto distrng = random->GetPoissonRng(kMean);
  sum = 0;
  for (uint64_t i = 0; i < kN; i++) {
    const int v = distrng.Sample();
    EXPECT_GE(v, 0);
    sum += v;
  }
  const double rng_mean = sum / static_cast<double>(kN);
  EXPECT_NEAR(rng_mean, static_cast<double>(kMean), 0.2);
}

// Integer now uses std::uniform_int_distribution so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence.
//   2. Range           – every sample lies in [0, max - 1].
TEST(RandomTest, Integer) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<unsigned> run1;
  for (uint64_t i = 1; i < 10; i++) {
    run1.push_back(random->Integer(static_cast<int>(i + 1)));
  }
  random->SetSeed(42);
  for (uint64_t i = 1; i < 10; i++) {
    EXPECT_EQ(run1[i - 1], random->Integer(static_cast<int>(i + 1)));
  }

  // --- 2. Range check: Integer(max) must be in [0, max - 1] -------------
  random->SetSeed(123);
  const int kMax = 7;
  for (uint64_t i = 0; i < 1000; i++) {
    const unsigned v = random->Integer(kMax);
    EXPECT_LE(v, static_cast<unsigned>(kMax - 1));
  }
}

// Circle now uses std::uniform_real_distribution + cos/sin so per-sample
// parity with TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence of points.
//   2. Radius invariant – every point lies on the circle of radius r.
//   3. Symmetry        – with N=10000 samples the empirical means of x and y
//                        are close to 0.
TEST(RandomTest, Circle) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  const real_t kR = static_cast<real_t>(3);
  
  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<MathArray<real_t, 2>> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Circle(kR));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    auto actual = random->Circle(kR);
    EXPECT_REAL_EQ(run1[i][0], actual[0]);
    EXPECT_REAL_EQ(run1[i][1], actual[1]);
  }

  // --- 2 & 3. Radius invariant + symmetry check -------------------------
  const uint64_t kN = 10000;
  random->SetSeed(123);
  double sum_x = 0;
  double sum_y = 0;
  for (uint64_t i = 0; i < kN; i++) {
    auto p = random->Circle(kR);
    const real_t radius =
        std::sqrt(p[0] * p[0] + p[1] * p[1]);
    EXPECT_NEAR(static_cast<double>(radius), static_cast<double>(kR),
                static_cast<double>(abs_error<real_t>::value));
    sum_x += p[0];
    sum_y += p[1];
  }
  const double mean_x = sum_x / static_cast<double>(kN);
  const double mean_y = sum_y / static_cast<double>(kN);
  EXPECT_NEAR(mean_x, 0.0, 0.1);
  EXPECT_NEAR(mean_y, 0.0, 0.1);
}

// Sphere now uses Gaussian-vector normalisation so per-sample parity with
// TRandom3 no longer holds.  We verify:
//   1. Reproducibility – same seed → same sequence of points.
//   2. Radius invariant – every point lies on the sphere of radius r.
//   3. Symmetry        – with N=10000 samples the empirical means of x, y, z
//                        are close to 0.
//   4. Distribution    – the empirical variance of z is close to r*r / 3
//                        (each coordinate of a uniform point on the sphere
//                        has variance r*r/3).
TEST(RandomTest, Sphere) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  const real_t kR = static_cast<real_t>(3);

  // --- 1. Reproducibility: same seed → identical sequence ---------------
  random->SetSeed(42);
  std::vector<MathArray<real_t, 3>> run1;
  for (uint64_t i = 0; i < 10; i++) {
    run1.push_back(random->Sphere(kR));
  }
  random->SetSeed(42);
  for (uint64_t i = 0; i < 10; i++) {
    auto actual = random->Sphere(kR);
    EXPECT_REAL_EQ(run1[i][0], actual[0]);
    EXPECT_REAL_EQ(run1[i][1], actual[1]);
    EXPECT_REAL_EQ(run1[i][2], actual[2]);
  }

  // --- 2, 3 & 4. Radius invariant + symmetry + variance check -----------
  const uint64_t kN = 10000;
  random->SetSeed(123);
  double sum_x = 0;
  double sum_y = 0;
  double sum_z = 0;
  double sum_z_sq = 0;
  for (uint64_t i = 0; i < kN; i++) {
    auto p = random->Sphere(kR);
    const real_t radius =
        std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    EXPECT_NEAR(static_cast<double>(radius), static_cast<double>(kR),
                static_cast<double>(abs_error<real_t>::value));
    sum_x += p[0];
    sum_y += p[1];
    sum_z += p[2];
    sum_z_sq += static_cast<double>(p[2]) * p[2];
  }
  const double mean_x = sum_x / static_cast<double>(kN);
  const double mean_y = sum_y / static_cast<double>(kN);
  const double mean_z = sum_z / static_cast<double>(kN);
  EXPECT_NEAR(mean_x, 0.0, 0.1);
  EXPECT_NEAR(mean_y, 0.0, 0.1);
  EXPECT_NEAR(mean_z, 0.0, 0.1);

  const double var_z =
      sum_z_sq / static_cast<double>(kN) - mean_z * mean_z;
  const double expected_var =
      static_cast<double>(kR) * static_cast<double>(kR) / 3.0;
  EXPECT_NEAR(var_z, expected_var, 0.2);
}
// IOTest for Random after the std::mt19937_64 migration.
//
// NOTE: mt_engine_ is marked //! (ROOT-transient) so its internal state is
// NOT written to disk.  After a BackupAndRestore round-trip the engine is
// re-default-constructed, which is deterministic but different from the
// pre-backup state.  Callers that need reproducibility across checkpoints
// must call SetSeed() again after restore.
//
// What we verify here:
//   a) The object serializes and deserializes without crashing.
//   b) Gaus() and Uniform() produce finite values after restore.
//   c) After re-seeding the restored object its output matches a freshly
//      seeded Random with the same seed (engine is working correctly).
#ifdef USE_DICT
TEST_F(IOTest, Random) {
  Random random;
  random.SetSeed(42);
  // Consume a few values so the pre-backup state is non-trivial.
  for (uint64_t i = 0; i < 10; i++) {
    (void)random.Gaus();
  }

  Random* restored;
  BackupAndRestore(random, &restored);

  // (b) Values must be finite – the engine must be functional after restore.
  for (uint64_t i = 0; i < 10; i++) {
    const real_t u = restored->Uniform(static_cast<real_t>(i),
                                       static_cast<real_t>(i + 2));
    EXPECT_TRUE(std::isfinite(static_cast<double>(u)));
    EXPECT_GE(u, static_cast<real_t>(i));
    EXPECT_LT(u, static_cast<real_t>(i + 2));

    const real_t g = restored->Gaus();
    EXPECT_TRUE(std::isfinite(static_cast<double>(g)));
  }

  // (c) Re-seeding the restored object must give the same sequence as a
  //     fresh Random seeded identically, proving the engine itself is intact.
  Random fresh;
  const uint64_t kReseed = 99;
  restored->SetSeed(kReseed);
  fresh.SetSeed(kReseed);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_REAL_EQ(fresh.Uniform(), restored->Uniform());
    EXPECT_REAL_EQ(fresh.Gaus(), restored->Gaus());
  }
}

TEST_F(IOTest, UserDefinedDistRng1D) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  auto ud_dist = [](const double* x, const double* param) { return sin(*x); };
  auto udd_rng = random->GetUserDefinedDistRng1D(ud_dist, {}, 0, 3);

  gRandom->SetSeed(42);
  std::vector<real_t> expected;
  for (int i = 0; i < 10; ++i) {
    expected.push_back(udd_rng.Sample());
  }

  UserDefinedDistRng1D* restored;
  BackupAndRestore(udd_rng, &restored);

  gRandom->SetSeed(42);
  std::vector<real_t> actual;
  for (int i = 0; i < 10; ++i) {
    actual.push_back(restored->Sample());
  }
  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_NEAR(expected[i], actual[i], 1e-6);
  }
}

#endif  // USE_DICT

}  // namespace bdm
