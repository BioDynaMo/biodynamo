// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "backend.h"
#include "biodynamo.h"
#include "cell.h"
#include "compile_time_param.h"
#include "gtest/gtest.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/param.h"
#include "simulation_implementation.h"
#include "unit/test_util.h"

namespace bdm {

BDM_CTPARAM(experimental::neuroscience) {
  BDM_CTPARAM_HEADER(experimental::neuroscience);

  using SimObjectTypes = CTList<Cell, experimental::neuroscience::NeuronSoma,
                                experimental::neuroscience::NeuriteElement>;
};

namespace experimental {
namespace neuroscience {

TEST(MechanicalInteraction, StraightxCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({1, 0, 0});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  std::array<double, 3> direction = {1, 0, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, StraightyCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 1, 0});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  std::array<double, 3> direction = {0, 1, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, StraightzCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  std::array<double, 3> direction = {0, 0, 1};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, DiagonalxyCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({1, 1, 0});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  std::array<double, 3> direction = {1, 1, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], ne_axis[1], abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, DiagonalxyzCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({1, 1, 1});
  ne->SetDiameter(1);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0.57735026918962584, abs_error<double>::value);

  std::array<double, 3> direction = {1, 1, 1};
  for (int i = 0; i < 37; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);

    ne_axis = ne->GetSpringAxis();

    EXPECT_NEAR(ne_axis[0], ne_axis[1], abs_error<double>::value);
    EXPECT_NEAR(ne_axis[0], ne_axis[2], abs_error<double>::value);
  }
}

TEST(MechanicalInteraction, DiagonalSpecialDirectionCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({1, 1, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0.57735026918962584, abs_error<double>::value);

  std::array<double, 3> direction = {2, 1, 1};

  for (int i = 0; i < 98; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);

    ne_axis = ne->GetSpringAxis();

    EXPECT_TRUE(std::round(1e9 * ne_axis[1]) == std::round(1e9 * ne_axis[2]));
  }
}

TEST(MechanicalInteraction, StraightCylinderGrowthObstacle) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({0, 0, 30});
  neuron2.SetMass(1);
  neuron2.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  std::array<double, 3> direction = {0, 0, 1};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, NotStraightCylinderGrowthObstacle) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({0, 0, 30});
  neuron2.SetMass(1);
  neuron2.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  std::array<double, 3> direction = {0.01, 0, 1};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);
  }

  ne_axis = ne->GetSpringAxis();
  ASSERT_GT(ne->GetMassLocation()[0], 5);
  ASSERT_TRUE(ne_axis[0] < 0.1);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
}

TEST(MechanicalInteraction, DoubleStraightCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 1, 0});
  auto ne2 = neuron.ExtendNewNeurite({1, 0, 0});
  ne->SetDiameter(2);
  ne2->SetDiameter(3);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();
  std::array<double, 3> ne_axis_2 = ne2->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  EXPECT_NEAR(ne_axis_2[0], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis_2[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis_2[2], 0, abs_error<double>::value);

  std::array<double, 3> direction = {0, 1, 0};
  std::array<double, 3> direction2 = {1, 0, 0};

  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    ne2->ElongateTerminalEnd(300, direction2);
    ne2->RunDiscretization();

    scheduler.Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();
      ne_axis_2 = ne2->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

      EXPECT_NEAR(ne_axis_2[1], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis_2[2], 0, abs_error<double>::value);
    }
  }
}

TEST(MechanicalInteraction, BifurcationCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  gErrorIgnoreLevel = kWarning;

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  std::array<double, 3> direction = {0, 0.5, 1};
  std::array<double, 3> direction2 = {0.5, 0, 1};

  for (int i = 0; i < 10; i++) {
    ne->ElongateTerminalEnd(100, {0, 0, 1});
    ne->RunDiscretization();
    scheduler.Simulate(1);
  }

  auto branches = ne->Bifurcate();
  auto branch_l = branches[0];
  auto branch_r = branches[1];

  for (int i = 0; i < 200; i++) {
    branch_r->ElongateTerminalEnd(100, direction);
    branch_r->RunDiscretization();
    branch_l->ElongateTerminalEnd(100, direction2);
    branch_l->RunDiscretization();
    scheduler.Simulate(1);
  }
  ne_axis = branch_l->GetSpringAxis();
  std::array<double, 3> ne_axis_2 = branch_r->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], ne_axis_2[1], abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], ne_axis_2[0], abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], ne_axis_2[2], abs_error<double>::value);
}

TEST(MechanicalInteraction, BranchCylinderGrowth) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis = ne->GetSpringAxis();

  std::array<double, 3> direction = {0, 0.5, 1};
  std::array<double, 3> direction2 = {0.5, 0, 1};

  for (int i = 0; i < 10; i++) {
    ne->ElongateTerminalEnd(100, {0, 0, 1});
    ne->RunDiscretization();
    scheduler.Simulate(1);
  }

  auto ne2 = ne->Branch(0.5, direction2);

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne2->ElongateTerminalEnd(100, direction2);
    ne->RunDiscretization();
    ne2->RunDiscretization();

    scheduler.Simulate(1);
  }

  ne_axis = ne->GetSpringAxis();
  std::array<double, 3> ne_axis_2 = ne2->GetSpringAxis();
  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis_2[1], 0, abs_error<double>::value);
}

TEST(MechanicalInteraction, getNeuronTest) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);
  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  auto ne2 = neuron.ExtendNewNeurite({1, 0, 0});

  Scheduler<> scheduler;

  for (int i = 0; i < 10; i++) {
    ne->ElongateTerminalEnd(100, ne->GetSpringAxis());
    ne->RunDiscretization();
    scheduler.Simulate(1);
  }

  auto ne3 = ne->Branch(0.5, {0.5, 0, 1});

  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({20, 0, 0});
  neuron2.SetMass(1);
  neuron2.SetDiameter(10);
  auto ne4 = neuron2.ExtendNewNeurite({0, 0, 1});

  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, ne->GetSpringAxis());
    ne3->ElongateTerminalEnd(100, ne3->GetSpringAxis());
    ne4->ElongateTerminalEnd(100, ne4->GetSpringAxis());
    ne->RunDiscretization();
    ne3->RunDiscretization();
    ne4->RunDiscretization();

    scheduler.Simulate(1);
  }

  EXPECT_TRUE(neuron.GetSoPtr() == ne->GetNeuronSomaOfNeurite());
  EXPECT_TRUE(ne->GetNeuronSomaOfNeurite() == ne2->GetNeuronSomaOfNeurite());
  EXPECT_TRUE(ne->GetNeuronSomaOfNeurite() == ne3->GetNeuronSomaOfNeurite());
  EXPECT_TRUE(neuron2->GetSoPtr() == ne4->GetNeuronSomaOfNeurite());
  EXPECT_FALSE(ne4->GetNeuronSomaOfNeurite() == ne3->GetNeuronSomaOfNeurite());
}

}  // end namespace neuroscience
}  // end namespace experimental
}  // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
