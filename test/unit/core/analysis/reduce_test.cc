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

#include "core/analysis/reduce.h"
#include <gtest/gtest.h>
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TEST(Reduce, Reduce) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  for (uint64_t i = 0; i < 2000; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  }

  auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
    *tl_result += bdm_static_cast<TestAgent*>(agent)->GetData();
  });
  SumReduction<uint64_t> combine_tl_results;
  auto result = Reduce(&sim, sum_data, combine_tl_results);
  EXPECT_EQ(1999000u, result);
}

// -----------------------------------------------------------------------------
TEST(Reduce, Reducer) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  for (uint64_t i = 0; i < 2000; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  }

  auto sum_data = [](Agent* agent, uint64_t* tl_result) {
    *tl_result += bdm_static_cast<TestAgent*>(agent)->GetData();
  };
  auto combine_tl_results = [](const SharedData<uint64_t>& tl_results) {
    uint64_t result = 0;
    for (auto& el : tl_results) {
      result += el;
    }
    return result;
  };

  // without post processing
  {
  Reducer<uint64_t> reducer(sum_data, combine_tl_results);
  rm->ForEachAgentParallel(reducer);
  auto result = reducer.GetResult();
  EXPECT_EQ(1999000u, result);
  }
  
  // with post processing
  {
    auto post_process = [](uint64_t result) {
      return result / 2;
    };
  Reducer<uint64_t> reducer(sum_data, combine_tl_results, post_process);
  rm->ForEachAgentParallel(reducer);
  auto result = reducer.GetResult();
  EXPECT_EQ(999500u, result);
  }

  // with different result data type and post processing
  {
    auto post_process = [](double result) {
      return result / 2.3;
    };
  Reducer<uint64_t, double> reducer(sum_data, combine_tl_results, post_process);
  rm->ForEachAgentParallel(reducer);
  auto result = reducer.GetResult();
  EXPECT_EQ(typeid(result), typeid(double));
  EXPECT_NEAR(869130.434782609, result, abs_error<double>::value);
  }
}

// -----------------------------------------------------------------------------
TEST(Reduce, Count) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  for (uint64_t i = 0; i < 2000; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  }

  auto data_lt_1000 = L2F([](Agent* agent) {
    return bdm_static_cast<TestAgent*>(agent)->GetData() < 1000;
  });
  auto result = Count(&sim, data_lt_1000);
  EXPECT_EQ(1000u, result);
}

// -----------------------------------------------------------------------------
TEST(Reduce, Counter) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  for (uint64_t i = 0; i < 2000; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  }

  auto data_lt_1000 = [](Agent* agent) {
    return bdm_static_cast<TestAgent*>(agent)->GetData() < 1000;
  };

  // without post processing
  {
    Counter<> counter(data_lt_1000);
    rm->ForEachAgentParallel(counter);
    auto result = counter.GetResult();
    EXPECT_EQ(1000u, result);
  }
  
  // with post processing
  {
    auto post_process = [](uint64_t result) {
      return result / 2;
    };
    Counter<> counter(data_lt_1000, post_process);
    rm->ForEachAgentParallel(counter);
    auto result = counter.GetResult();
    EXPECT_EQ(500u, result);
  }

  // with different result data type and post processing
  {
    auto post_process = [](double result) {
      return result / 2.3;
    };
    Counter<double> counter(data_lt_1000, post_process);
    rm->ForEachAgentParallel(counter);
    auto result = counter.GetResult();
    EXPECT_EQ(typeid(result), typeid(double));
    EXPECT_NEAR(434.782608696, result, abs_error<double>::value);
  }
}

}  // namespace experimental
}  // namespace bdm
