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

#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/optimization_param_type/log_range_param.h"
#include "core/multi_simulation/optimization_param_type/range_param.h"
#include "core/multi_simulation/optimization_param_type/set_param.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

TEST(DynamicLoopTest, DynamicLoop) {
  std::vector<OptimizationParamType*> ranges = {
      new RangeParam("a", -1.5, 1.5, 0.5), new LogRangeParam("b", 2, 0, 10, 5),
      new SetParam("c", {0, 1})};

  // Write out the nested for loops and extract the expected results
  std::vector<std::vector<double>> expected;
  for (int j = 0; j < 2; j++) {
    for (double lrp = 0; lrp < 11; lrp += 5) {
      for (double rp = -1.5; rp < 1.6; rp += 0.5) {
        expected.push_back({rp, std::pow(2, lrp), static_cast<double>(j)});
      }
    }
  }

  std::vector<double> paramset(3);

  int it = 0;
  auto lambda = [&](std::vector<uint32_t> slots) {
    int i = 0;
    for (auto& param : paramset) {
      param = ranges[i]->GetValue(slots[i]);
      i++;
    }

    EXPECT_VEC_NEAR(expected[it], paramset);
    it++;
  };

  DynamicNestedLoop(ranges, lambda);
}

}  // namespace experimental
}  // namespace bdm
