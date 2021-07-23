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

#include <gtest/gtest.h>

#include "core/multi_simulation/algorithm/algorithm.h"
#include "core/multi_simulation/algorithm/algorithm_registry.h"
#include "unit/test_util/test_util.h"

namespace bdm {

// -----------------------------------------------------------------------------
TEST(AlgorithmTest, Registry) {
  auto* algo_registry = AlgorithmRegistry::GetInstance();

  EXPECT_NE(nullptr, algo_registry);

  auto* param_sweep = algo_registry->GetAlgorithm("ParameterSweep");
  EXPECT_NE(nullptr, param_sweep);
  auto* pso = algo_registry->GetAlgorithm("ParticleSwarm");
  EXPECT_NE(nullptr, pso);
}

// -----------------------------------------------------------------------------
struct MyAlgorithm : public Algorithm {
  BDM_ALGO_HEADER();
  void operator()(
      const std::function<void(Param*, TimeSeries*)>& send_params_to_worker,
      Param* default_param) {}
};
BDM_REGISTER_ALGO(MyAlgorithm)

TEST(AlgorithmTest, AddAlgorithm) {
  auto* algo_registry = AlgorithmRegistry::GetInstance();
  auto* my_algo = algo_registry->GetAlgorithm("MyAlgorithm");
  EXPECT_NE(nullptr, my_algo);
}

// -----------------------------------------------------------------------------
TEST(AlgorithmTest, UnregisteredAlgorithm) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        auto* algo_registry = AlgorithmRegistry::GetInstance();
        auto* nen = algo_registry->GetAlgorithm("NonExistingName");
        EXPECT_EQ(nullptr, nen);
      },
      ".*Algorithm not found in registry*");
}

}  // namespace bdm
