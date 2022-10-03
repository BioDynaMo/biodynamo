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
  void Step(real_t dt) final { n_steps_ += 1; }
  int GetNSteps() { return n_steps_; }

 private:
  int n_steps_ = 0;
};

inline void CellFactory(const std::vector<Real3>& positions) {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->Reserve(positions.size());
  for (size_t i = 0; i < positions.size(); i++) {
    Cell* cell = new Cell({positions[i][0], positions[i][1], positions[i][2]});
    cell->SetDiameter(30);
    rm->AddAgent(cell);
  }
};

TEST(ContinuumTest, AsynchronousUpdates) {
  const double time_step = 0.01;
  auto set_param = [&](Param* param) {
    param->simulation_time_step = time_step;
  };
  Simulation simulation(TEST_NAME, set_param);
  auto* param = simulation.GetParam();
  std::vector<Real3> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(positions);

  auto* tf1 = new TestField();
  real_t ts1 = 0.05;
  tf1->SetContinuumId(0);
  tf1->SetContinuumName("TestField1");
  tf1->SetTimeStep(ts1);

  auto* tf2 = new TestField();
  real_t ts2 = 0.002;
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

  // The frequency of 2 triggest the operation every 2 time steps, e.g. 10 times
  // for a total simulation time of 19/20*0.01=0.19.

  // 1.1 TestField1 steps: 0.19 / 0.05 = 3.8 -> 3
  const int expected_n_steps_tf1 = 3;
  // 1.2 Expected time trivially obtained by multiplying the number of steps
  //     with the time step
  const double expected_time_tf1 = expected_n_steps_tf1 * ts1;
  // 2.1 TestField2 steps: 0.19 / 0.002 = 95 -> 95
  const int expected_n_steps_tf2 = 95;
  // 2.2. Simulated time is trivially computed as time steps * time step size.
  const double expected_time_tf2 = expected_n_steps_tf2 * ts2;
  // 3.1 Without time step, the field will by default adapt a time step of
  //      simulation_time_ for the first step and  simulation_time_step *
  //      frequency for all further steps.
  const int expected_n_steps_tf3 = 10;
  // 3.2 According to previous point, the expected time is 0.01 * 1 + 0.02 * 9 =
  //     0.19.
  const double expected_time_tf3 = 0.19;

  // Check time step
  EXPECT_EQ(tf1->GetTimeStep(), ts1);
  EXPECT_EQ(tf2->GetTimeStep(), ts2);
  EXPECT_EQ(tf3->GetTimeStep(), param->simulation_time_step * frequency);
  // Check simulated steps
  EXPECT_EQ(expected_n_steps_tf1, tf1->GetNSteps());
  EXPECT_EQ(expected_n_steps_tf2, tf2->GetNSteps());
  EXPECT_EQ(expected_n_steps_tf3, tf3->GetNSteps());
  // Check simulated time
  EXPECT_REAL_EQ(expected_time_tf1, tf1->GetSimulatedTime());
  EXPECT_REAL_EQ(expected_time_tf2, tf2->GetSimulatedTime());
  EXPECT_REAL_EQ(expected_time_tf3, tf3->GetSimulatedTime());
}

}  // namespace bdm
