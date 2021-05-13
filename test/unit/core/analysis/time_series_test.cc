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

#include "core/analysis/time_series.h"
#include <gtest/gtest.h>
#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddExistingData) {
  TimeSeries ts;
  EXPECT_EQ(0u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4});
  EXPECT_EQ(1u, ts.Size());
  EXPECT_TRUE(ts.Contains("my-entry"));
  const auto& xvals = ts.GetXValues("my-entry");
  EXPECT_EQ(2u, xvals.size());
  EXPECT_NEAR(1.0, xvals[0], abs_error<double>::value);
  EXPECT_NEAR(2.0, xvals[1], abs_error<double>::value);
  const auto& yvals = ts.GetYValues("my-entry");
  EXPECT_EQ(2u, yvals.size());
  EXPECT_NEAR(3.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddExistingTwice) {
  TimeSeries ts;
  ts.Add("my-entry", {1, 2}, {3, 4});
  EXPECT_EQ(1u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4});
  EXPECT_EQ(1u, ts.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddOtherTs) {
  TimeSeries ts;
  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.Add("my-entry2", {1, 2, 3}, {4, 5, 6});
  EXPECT_EQ(2u, ts.Size());

  TimeSeries ts2;
  EXPECT_EQ(0u, ts2.Size());
  ts2.Add(ts, "suffix");
  EXPECT_EQ(2u, ts2.Size());
  EXPECT_TRUE(ts2.Contains("my-entry-suffix"));
  EXPECT_TRUE(ts2.Contains("my-entry2-suffix"));

  // check entries for my-entry
  const auto& xvals = ts2.GetXValues("my-entry-suffix");
  EXPECT_EQ(2u, xvals.size());
  EXPECT_NEAR(1.0, xvals[0], abs_error<double>::value);
  EXPECT_NEAR(2.0, xvals[1], abs_error<double>::value);
  const auto& yvals = ts2.GetYValues("my-entry-suffix");
  EXPECT_EQ(2u, yvals.size());
  EXPECT_NEAR(3.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddOtherTsToItself) {
  TimeSeries ts;
  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.Add("my-entry2", {1, 2, 3}, {4, 5, 6});
  EXPECT_EQ(2u, ts.Size());
  ts.Add(ts, "suffix");
  EXPECT_EQ(2u, ts.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddOtherTsTwice) {
  TimeSeries ts;
  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.Add("my-entry2", {1, 2, 3}, {4, 5, 6});

  TimeSeries ts2;
  EXPECT_EQ(0u, ts2.Size());
  ts2.Add(ts, "suffix");
  EXPECT_EQ(2u, ts2.Size());
  ts2.Add(ts, "suffix");
  EXPECT_EQ(2u, ts2.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddCollectorAndUpdate) {
  Simulation sim(TEST_NAME);

  // cells will divide in every step -> the number of agents will double
  // each iteration
  StatelessBehavior rapid_division(
      [](Agent* agent) { bdm_static_cast<Cell*>(agent)->Divide(0.5); });
  rapid_division.AlwaysCopyToNew();
  auto* cell = new Cell();
  cell->AddBehavior(rapid_division.NewCopy());
  sim.GetResourceManager()->AddAgent(cell);

  auto* ts = sim.GetTimeSeries();
  auto get_num_agents = [](Simulation* sim) {
    return static_cast<double>(sim->GetResourceManager()->GetNumAgents());
  };
  ts->AddCollector("num-agents", get_num_agents);
  EXPECT_EQ(1u, ts->Size());
  EXPECT_TRUE(ts->Contains("num-agents"));

  sim.GetScheduler()->Simulate(3);

  auto* param = sim.GetParam();
  // check entries for my-entry
  const auto& xvals = ts->GetXValues("num-agents");
  EXPECT_EQ(3u, xvals.size());
  for(uint64_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(i * param->simulation_time_step, xvals[i], abs_error<double>::value);
  }  
  const auto& yvals = ts->GetYValues("num-agents");
  EXPECT_EQ(3u, yvals.size());
  EXPECT_NEAR(2.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
  EXPECT_NEAR(8.0, yvals[2], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, StoreAndLoad) {
  TimeSeries ts;
  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.Save("ts.root");

  TimeSeries* restored = nullptr;

  TimeSeries::Load("ts.root", &restored);
  ASSERT_TRUE(restored != nullptr);

  EXPECT_EQ(1u, restored->Size());
  EXPECT_TRUE(restored->Contains("my-entry"));
  const auto& xvals = restored->GetXValues("my-entry");
  EXPECT_EQ(2u, xvals.size());
  EXPECT_NEAR(1.0, xvals[0], abs_error<double>::value);
  EXPECT_NEAR(2.0, xvals[1], abs_error<double>::value);
  const auto& yvals = restored->GetYValues("my-entry");
  EXPECT_EQ(2u, yvals.size());
  EXPECT_NEAR(3.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
}

}  // namespace experimental
}  // namespace bdm
