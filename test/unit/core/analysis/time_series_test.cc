// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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
#include <TMath.h>
#include <gtest/gtest.h>
#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "core/behavior/stateless_behavior.h"
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
TEST(TimeSeries, AddExistingDataWithError) {
  TimeSeries ts;
  EXPECT_EQ(0u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4}, {0.1, 0.2});
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
  const auto& yel = ts.GetYErrorLow("my-entry");
  EXPECT_EQ(2u, yel.size());
  EXPECT_NEAR(0.1, yel[0], abs_error<double>::value);
  EXPECT_NEAR(0.2, yel[1], abs_error<double>::value);
  const auto& yeh = ts.GetYErrorHigh("my-entry");
  EXPECT_EQ(2u, yeh.size());
  EXPECT_NEAR(0.1, yeh[0], abs_error<double>::value);
  EXPECT_NEAR(0.2, yeh[1], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddExistingDataWithErrorAsym) {
  TimeSeries ts;
  EXPECT_EQ(0u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4}, {0.1, 0.2}, {0.3, 0.4});
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
  const auto& yel = ts.GetYErrorLow("my-entry");
  EXPECT_EQ(2u, yel.size());
  EXPECT_NEAR(0.1, yel[0], abs_error<double>::value);
  EXPECT_NEAR(0.2, yel[1], abs_error<double>::value);
  const auto& yeh = ts.GetYErrorHigh("my-entry");
  EXPECT_EQ(2u, yeh.size());
  EXPECT_NEAR(0.3, yeh[0], abs_error<double>::value);
  EXPECT_NEAR(0.4, yeh[1], abs_error<double>::value);
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
  for (uint64_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(i * param->simulation_time_step, xvals[i],
                abs_error<double>::value);
  }
  const auto& yvals = ts->GetYValues("num-agents");
  EXPECT_EQ(3u, yvals.size());
  EXPECT_NEAR(2.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
  EXPECT_NEAR(8.0, yvals[2], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddCollectorXYAndUpdate) {
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
  auto xcollector = [](Simulation* sim) {
    return sim->GetScheduler()->GetSimulatedSteps() + 3.0;
  };
  ts->AddCollector("num-agents", get_num_agents, xcollector);
  EXPECT_EQ(1u, ts->Size());
  EXPECT_TRUE(ts->Contains("num-agents"));

  sim.GetScheduler()->Simulate(3);

  // check entries for my-entry
  const auto& xvals = ts->GetXValues("num-agents");
  EXPECT_EQ(3u, xvals.size());
  for (uint64_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(i + 3, xvals[i], abs_error<double>::value);
  }
  const auto& yvals = ts->GetYValues("num-agents");
  EXPECT_EQ(3u, yvals.size());
  EXPECT_NEAR(2.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
  EXPECT_NEAR(8.0, yvals[2], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AddCollectorReducerAndUpdate) {
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
  auto agent_diam_gt_0 = [](Agent* a) { return a->GetDiameter() > 0.; };
  auto xcollector = [](Simulation* sim) {
    return sim->GetScheduler()->GetSimulatedSteps() + 3.0;
  };
  auto* counter = new Counter<double>(agent_diam_gt_0);
  ts->AddCollector("agents-diam-gt-0", counter, xcollector);
  EXPECT_EQ(1u, ts->Size());
  EXPECT_TRUE(ts->Contains("agents-diam-gt-0"));

  sim.GetScheduler()->Simulate(3);

  // check entries for my-entry
  const auto& xvals = ts->GetXValues("agents-diam-gt-0");
  EXPECT_EQ(3u, xvals.size());
  for (uint64_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(i + 3, xvals[i], abs_error<double>::value);
  }
  const auto& yvals = ts->GetYValues("agents-diam-gt-0");
  EXPECT_EQ(3u, yvals.size());
  EXPECT_NEAR(2.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);
  EXPECT_NEAR(8.0, yvals[2], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, StoreAndLoad) {
  Simulation sim(TEST_NAME);
  sim.GetResourceManager()->AddAgent(new Cell());
  sim.GetResourceManager()->AddAgent(new Cell());

  TimeSeries ts;
  auto ycollector = [](Simulation* sim) { return 4.0; };
  auto xcollector = [](Simulation* sim) { return 5.0; };
  ts.AddCollector("collect", ycollector, xcollector);

  auto d_gt_0 = [](Agent* a) { return a->GetDiameter() > 0; };
  auto* counter = new Counter<double>(d_gt_0);
  ts.AddCollector("collect1", counter, xcollector);

  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.Save("ts.root");

  TimeSeries* restored = nullptr;

  TimeSeries::Load("ts.root", &restored);
  ASSERT_TRUE(restored != nullptr);

  EXPECT_EQ(3u, restored->Size());
  EXPECT_TRUE(restored->Contains("my-entry"));
  EXPECT_TRUE(restored->Contains("collect"));
  EXPECT_TRUE(restored->Contains("collect1"));

  const auto& xvals = restored->GetXValues("my-entry");
  EXPECT_EQ(2u, xvals.size());
  EXPECT_NEAR(1.0, xvals[0], abs_error<double>::value);
  EXPECT_NEAR(2.0, xvals[1], abs_error<double>::value);
  const auto& yvals = restored->GetYValues("my-entry");
  EXPECT_EQ(2u, yvals.size());
  EXPECT_NEAR(3.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(4.0, yvals[1], abs_error<double>::value);

  // check if collector has been restored correctly.
  restored->Update();
  {
    const auto& xvals1 = restored->GetXValues("collect");
    EXPECT_EQ(1u, xvals1.size());
    EXPECT_NEAR(5.0, xvals1[0], abs_error<double>::value);
    const auto& yvals1 = restored->GetYValues("collect");
    EXPECT_EQ(1u, yvals1.size());
    EXPECT_NEAR(4.0, yvals1[0], abs_error<double>::value);
  }
  {
    const auto& xvals1 = restored->GetXValues("collect1");
    EXPECT_EQ(1u, xvals1.size());
    EXPECT_NEAR(5.0, xvals1[0], abs_error<double>::value);
    const auto& yvals1 = restored->GetYValues("collect1");
    EXPECT_EQ(1u, yvals1.size());
    EXPECT_NEAR(2.0, yvals1[0], abs_error<double>::value);
  }
  delete restored;
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, StoreJson) {
  TimeSeries ts;

  auto collect_function = [](Simulation* sim) { return 4.0; };
  ts.AddCollector("collect", collect_function);

  ts.Add("my-entry", {1, 2}, {3, 4});
  ts.SaveJson("ts.json");
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, MergeNullptr) {
  std::vector<TimeSeries> tss(2);
  tss[0].Add("entry-0", {}, {});
  tss[1].Add("entry-0", {}, {});

  TimeSeries::Merge(nullptr, tss,
                    [](const std::vector<double>& all_y_values, double* y,
                       double* el, double* eh) {});
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, MergeMissingEntries) {
  std::vector<TimeSeries> tss(2);
  tss[0].Add("entry-0", {}, {});
  tss[1].Add("entry-0", {}, {});
  tss[1].Add("entry-1", {}, {});

  TimeSeries merged;
  TimeSeries::Merge(&merged, tss,
                    [](const std::vector<double>& all_y_values, double* y,
                       double* el, double* eh) {});

  EXPECT_EQ(0u, merged.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, MergeDifferentNumberOfDataEntries) {
  std::vector<TimeSeries> tss(2);
  tss[0].Add("entry-0", {1, 2}, {3, 4});
  tss[1].Add("entry-0", {1, 2, 3}, {5, 6, 7});

  TimeSeries merged;
  TimeSeries::Merge(&merged, tss,
                    [](const std::vector<double>& all_y_values, double* y,
                       double* el, double* eh) {});

  EXPECT_EQ(0u, merged.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, MergeDifferentNumberOfXValues) {
  std::vector<TimeSeries> tss(2);
  tss[0].Add("entry-0", {1, 3}, {3, 4});
  tss[1].Add("entry-0", {1, 2}, {6, 7});

  TimeSeries merged;
  TimeSeries::Merge(&merged, tss,
                    [](const std::vector<double>& all_y_values, double* y,
                       double* el, double* eh) {});

  EXPECT_EQ(0u, merged.Size());
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, Merge) {
  std::vector<TimeSeries> tss(3);
  tss[0].Add("entry-0", {1, 2}, {2, 5});
  tss[1].Add("entry-0", {1, 2}, {4, 8});
  tss[2].Add("entry-0", {1, 2}, {1, 13});

  TimeSeries merged;
  TimeSeries::Merge(
      &merged, tss,
      [](const std::vector<double>& all_y_values, double* y, double* el,
         double* eh) {
        *y = TMath::Median(all_y_values.size(), all_y_values.data());
        *el = *y - *TMath::LocMin(all_y_values.begin(), all_y_values.end());
        *eh = *TMath::LocMax(all_y_values.begin(), all_y_values.end()) - *y;
      });

  EXPECT_EQ(1u, merged.Size());
  const auto& xvals = merged.GetXValues("entry-0");
  EXPECT_EQ(2u, xvals.size());
  EXPECT_NEAR(1.0, xvals[0], abs_error<double>::value);
  EXPECT_NEAR(2.0, xvals[1], abs_error<double>::value);
  const auto& yvals = merged.GetYValues("entry-0");
  EXPECT_EQ(2u, yvals.size());
  EXPECT_NEAR(2.0, yvals[0], abs_error<double>::value);
  EXPECT_NEAR(8.0, yvals[1], abs_error<double>::value);
  const auto& el = merged.GetYErrorLow("entry-0");
  EXPECT_EQ(2u, el.size());
  EXPECT_NEAR(1.0, el[0], abs_error<double>::value);
  EXPECT_NEAR(3.0, el[1], abs_error<double>::value);
  const auto& eh = merged.GetYErrorHigh("entry-0");
  EXPECT_EQ(2u, eh.size());
  EXPECT_NEAR(2.0, eh[0], abs_error<double>::value);
  EXPECT_NEAR(5.0, eh[1], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, AssignmentOperator) {
  TimeSeries ts;
  EXPECT_EQ(0u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4});

  TimeSeries ts2;
  ts2 = ts;
  EXPECT_EQ(1u, ts.Size());
  EXPECT_TRUE(ts.Contains("my-entry"));
  EXPECT_EQ(1u, ts2.Size());
  EXPECT_TRUE(ts2.Contains("my-entry"));
}

// -----------------------------------------------------------------------------
TEST(TimeSeries, MoveAssignmentOperator) {
  TimeSeries ts;
  EXPECT_EQ(0u, ts.Size());
  ts.Add("my-entry", {1, 2}, {3, 4});

  TimeSeries ts2;
  ts2 = std::move(ts);
  EXPECT_EQ(0u, ts.Size());
  EXPECT_EQ(1u, ts2.Size());
  EXPECT_TRUE(ts2.Contains("my-entry"));
}

}  // namespace experimental
}  // namespace bdm
