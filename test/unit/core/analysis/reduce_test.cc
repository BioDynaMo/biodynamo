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
#include "unit/test_util/test_util.h"
#include "unit/test_util/test_agent.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/simulation.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TEST(Reduce, Reduce) {
  Simulation sim(TEST_NAME);
  auto*rm = sim.GetResourceManager();

  for(uint64_t i = 0; i < 10; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  } 
  
  auto sum_data = L2F([](Agent* agent, uint64_t* tl_result){ 
      *tl_result += bdm_static_cast<TestAgent*>(agent)->GetData(); 
      });
  SumReduction<uint64_t> combine_tl_results;
  auto result = Reduce(&sim, sum_data, combine_tl_results); 
  EXPECT_EQ(45u, result);
}

// -----------------------------------------------------------------------------
TEST(Reduce, Count) {
  Simulation sim(TEST_NAME);
  auto*rm = sim.GetResourceManager();

  for(uint64_t i = 0; i < 10; ++i) {
    auto* a = new TestAgent();
    a->SetData(i);
    rm->AddAgent(a);
  } 
  
  auto data_lt_5 = L2F([](Agent* agent){ 
      return bdm_static_cast<TestAgent*>(agent)->GetData() < 5; 
      });
  auto result = Count(&sim, data_lt_5); 
  EXPECT_EQ(5u, result);
}

}  // namespace experimental
}  // namespace bdm
