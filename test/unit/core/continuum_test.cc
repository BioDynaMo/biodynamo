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

#include "core/agent/cell.h"
#include "core/diffusion/continuum_interface.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

class TestField : public Continuum {
 public:
  void Initialize() final {}
  void Update() final {}
  void Step(double dt) final { n_steps_ += 1; }
  int GetNSteps() { return n_steps_; }

 private:
  int n_steps_ = 0;
};

inline void CellFactory(const std::vector<Double3>& positions) {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->Reserve(positions.size());
  for (size_t i = 0; i < positions.size(); i++) {
    Cell* cell = new Cell({positions[i][0], positions[i][1], positions[i][2]});
    cell->SetDiameter(30);
    rm->AddAgent(cell);
  }
};

TEST(ContinuumTest, AsynchronousUpdates) {
  Simulation simulation(TEST_NAME);
  std::vector<Double3> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(positions);

  auto* tf1 = new TestField();
  double ts1 = 0.05;
  tf1->SetContinuumId(0);
  tf1->SetContinuumName("TestField1");
  tf1->SetTimeStep(ts1);

  auto* tf2 = new TestField();
  double ts2 = 0.002;
  tf2->SetContinuumId(1);
  tf2->SetContinuumName("TestField2");
  tf2->SetTimeStep(ts2);

  auto* tf3 = new TestField();
  tf3->SetContinuumId(2);
  tf3->SetContinuumName("TestField2");

  auto* rm = simulation.GetResourceManager();
  rm->AddContinuum(tf1);
  rm->AddContinuum(tf2);
  rm->AddContinuum(tf3);

  uint64_t n_sim_steps = 20;
  size_t frequency = 2;
  auto* scheduler = simulation.GetScheduler();
  auto* op = scheduler->GetOps("continuum")[0];
  op->frequency_ = frequency;
  simulation.GetScheduler()->Simulate(n_sim_steps);

  auto* param = simulation.GetParam();

  // The first step in the simulation is not included in the number of steps
  // performed by the TestFields.

  EXPECT_EQ(param->simulation_time_step * n_sim_steps / ts1, tf1->GetNSteps());
  EXPECT_EQ(param->simulation_time_step * n_sim_steps / ts2, tf2->GetNSteps());
  EXPECT_EQ(n_sim_steps / frequency, tf3->GetNSteps());
  EXPECT_DOUBLE_EQ(n_sim_steps * param->simulation_time_step,
                   tf1->GetSimulatedTime());
  EXPECT_DOUBLE_EQ(n_sim_steps * param->simulation_time_step,
                   tf2->GetSimulatedTime());
  EXPECT_DOUBLE_EQ(n_sim_steps * param->simulation_time_step,
                   tf3->GetSimulatedTime());
}

}  // namespace bdm
