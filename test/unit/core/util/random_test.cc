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
#include <Math/DistFunc.h>
#include <TF1.h>
#include <TRandom3.h>
#include <gtest/gtest.h>
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(RandomTest, Uniform) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(), random->Uniform());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i), random->Uniform(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i, i + 2), random->Uniform(i, i + 2));
  }

  auto distrng = random->GetUniformRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(3, 4), distrng.Sample());
  }
}

TEST(RandomTest, UniformArray) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  auto result = random->UniformArray<5>();
  for (uint64_t i = 0; i < 5; i++) {
    EXPECT_EQ(reference.Uniform(), result[i]);
  }

  auto result1 = random->UniformArray<2>(8.3);
  for (uint64_t i = 0; i < 2; i++) {
    EXPECT_EQ(reference.Uniform(8.3), result1[i]);
  }

  auto result2 = random->UniformArray<12>(5.1, 9.87);
  for (uint64_t i = 0; i < 12; i++) {
    EXPECT_EQ(reference.Uniform(5.1, 9.87), result2[i]);
  }
}

TEST(RandomTest, Gaus) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(), random->Gaus());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(i), random->Gaus(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(i, i + 2), random->Gaus(i, i + 2));
  }

  auto distrng = random->GetGausRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(3, 4), distrng.Sample());
  }
}

TEST(RandomTest, Exp) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Exp(i), random->Exp(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Exp(i + 2), random->Exp(i + 2));
  }

  auto distrng = random->GetExpRng(123);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Exp(123), distrng.Sample());
  }
}

TEST(RandomTest, Landau) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Landau(), random->Landau());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Landau(i), random->Landau(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Landau(i, i + 2), random->Landau(i, i + 2));
  }

  auto distrng = random->GetLandauRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Landau(3, 4), distrng.Sample());
  }
}

TEST(RandomTest, PoissonD) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.PoissonD(i), random->PoissonD(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.PoissonD(i + 2), random->PoissonD(i + 2));
  }

  auto distrng = random->GetPoissonDRng(123);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.PoissonD(123), distrng.Sample());
  }
}

TEST(RandomTest, BreitWigner) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.BreitWigner(), random->BreitWigner());
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.BreitWigner(i), random->BreitWigner(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.BreitWigner(i, i + 2), random->BreitWigner(i, i + 2));
  }

  auto distrng = random->GetBreitWignerRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.BreitWigner(3, 4), distrng.Sample());
  }
}

TEST(RandomTest, UserDefinedDistRng) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  auto function = [](const double* x, const double* params) {
    return ROOT::Math::lognormal_pdf(*x, params[0], params[1]);
  };
  double min = 1;
  double max = 4;
  TF1 reference("", function, min, max, 2);
  reference.SetParameters(1.1, 1.2);

  // workaround until update to 6.24
  gRandom->SetSeed(42);
  std::vector<double> expected;
  for (uint64_t i = 0; i < 10; i++) {
    expected.push_back(reference.GetRandom(min, max));
  }

  gRandom->SetSeed(42);
  auto distrng = random->GetUserDefinedDistRng(function, {1.1, 1.2}, min, max);
  std::vector<double> actual;
  for (uint64_t i = 0; i < 10; i++) {
    actual.push_back(distrng.Sample());
  }
  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_NEAR(expected[i], actual[i], abs_error<double>::value);
  }
}

TEST(RandomTest, Binomial) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Binomial(i, i + 2), random->Binomial(i, i + 2));
  }

  auto distrng = random->GetBinomialRng(3, 4);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Binomial(3, 4), distrng.Sample());
  }
}

TEST(RandomTest, Poisson) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Poisson(i), random->Poisson(i));
  }

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Poisson(i + 2), random->Poisson(i + 2));
  }

  auto distrng = random->GetPoissonRng(123);
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Poisson(123), distrng.Sample());
  }
}

TEST(RandomTest, Integer) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 1; i < 10; i++) {
    EXPECT_EQ(reference.Integer(i), random->Integer(i));
  }
}

TEST(RandomTest, Circle) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 1; i < 10; i++) {
    double expected_x = 0;
    double expected_y = 0;
    reference.Circle(expected_x, expected_y, i);

    auto actual = random->Circle(i);
    EXPECT_EQ(expected_x, actual[0]);
    EXPECT_EQ(expected_y, actual[1]);
  }
}

TEST(RandomTest, Sphere) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();
  TRandom3 reference;

  random->SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 1; i < 10; i++) {
    double expected_x = 0;
    double expected_y = 0;
    double expected_z = 0;
    reference.Sphere(expected_x, expected_y, expected_z, i);

    auto actual = random->Sphere(i);
    EXPECT_EQ(expected_x, actual[0]);
    EXPECT_EQ(expected_y, actual[1]);
    EXPECT_EQ(expected_z, actual[2]);
  }
}

#ifdef USE_DICT
TEST_F(IOTest, Random) {
  Random random;
  TRandom3 reference;

  random.SetSeed(42);
  reference.SetSeed(42);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(), random.Gaus());
  }

  Random* restored;
  BackupAndRestore(random, &restored);

  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i, i + 2), random.Uniform(i, i + 2));
  }
}

TEST_F(IOTest, UserDefinedDistRng) {
  Simulation simulation(TEST_NAME);
  auto* random = simulation.GetRandom();

  auto ud_dist = [](const double* x, const double* param) { return sin(*x); };
  auto udd_rng = random->GetUserDefinedDistRng(ud_dist, {}, 0, 3);

  gRandom->SetSeed(42);
  std::vector<double> expected;
  for (int i = 0; i < 10; ++i) {
    expected.push_back(udd_rng.Sample());
  }

  UserDefinedDistRng* restored;
  BackupAndRestore(udd_rng, &restored);

  gRandom->SetSeed(42);
  std::vector<double> actual;
  for (int i = 0; i < 10; ++i) {
    actual.push_back(restored->Sample());
  }
  for (size_t i = 0; i < actual.size(); i++) {
    EXPECT_NEAR(expected[i], actual[i], 1e-6);
  }
}

#endif  // USE_DICT

}  // namespace bdm
