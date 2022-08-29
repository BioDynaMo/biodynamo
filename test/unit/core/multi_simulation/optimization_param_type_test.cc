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

#include <gtest/gtest.h>

#include "core/multi_simulation/optimization_param_type/log_range_param.h"
#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"
#include "core/multi_simulation/optimization_param_type/particle_swarm_param.h"
#include "core/multi_simulation/optimization_param_type/range_param.h"
#include "core/multi_simulation/optimization_param_type/set_param.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(OptimizationParamTypeTest, RangeParam) {
  RangeParam rp("TestParam", 0, 10, 1);
  EXPECT_EQ(11u, rp.GetNumElements());

  for (uint32_t i = 0; i < rp.GetNumElements(); i++) {
    EXPECT_EQ(i, rp.GetValue(i));
  }

  RangeParam rp2("TestParam", -3.5, 3.5, 0.5);
  EXPECT_EQ(15u, rp2.GetNumElements());
  EXPECT_EQ(0, rp2.GetValue(7));
  EXPECT_EQ(-3.5, rp2.GetValue(0));
}

TEST(OptimizationParamTypeTest, InvertedBounds) {
  EXPECT_DEATH_IF_SUPPORTED(
      { RangeParam rp("TestParam", 10, 1, 1); },
      ".*with a lower_bound value higher than upper_bound*");

  EXPECT_DEATH_IF_SUPPORTED(
      { LogRangeParam rp("TestParam", 2, 10, 1, 1); },
      ".*with a lower_bound value higher than upper_bound*");

  EXPECT_DEATH_IF_SUPPORTED(
      { ParticleSwarmParam rp("TestParam", 10, 1, 1); },
      ".*with a lower_bound value higher than upper_bound*");
}

TEST(OptimizationParamTypeTest, LogRangeParam) {
  LogRangeParam lrp("TestParam", 2, 0, 10, 1);
  EXPECT_EQ(11u, lrp.GetNumElements());

  for (uint32_t i = 0; i < lrp.GetNumElements(); i++) {
    EXPECT_EQ(std::pow(2, i), lrp.GetValue(i));
  }
}

TEST(OptimizationParamTypeTest, SetParam) {
  std::vector<real_t> vals = {1, 12, 4, 4, 5, 91, -2.4, -93};
  SetParam sp("TestParam", vals);
  EXPECT_EQ(8u, sp.GetNumElements());

  for (uint32_t i = 0; i < sp.GetNumElements(); i++) {
    EXPECT_EQ(vals[i], sp.GetValue(i));
  }
}

}  // namespace bdm
