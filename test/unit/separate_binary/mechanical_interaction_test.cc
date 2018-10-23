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

// as the dendrite grows exactly at the center of the second cells
// growth force/direction and repulsive force/direction are equal
// so the dendrite stop growing
TEST(MechanicalInteraction, StraightCylinderGrowthObstacle) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
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
  neuron.SetDiameter(10);

  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({0, 0, 30});
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

    ne_axis = ne->GetSpringAxis();

    EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  }

  ne_axis = ne->GetSpringAxis();
  EXPECT_GT(ne->GetMassLocation()[0], 5);
  EXPECT_GT(ne_axis[0], 0);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
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

TEST(MechanicalInteraction, BifurcateCylinderRandomGrowth) {
  auto set_param = [](auto* param) { param->neurite_max_length_ = 2; };
  Simulation<> simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* random = simulation.GetRandom();

  auto neuron = rm->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> ne_axis;
  std::array<double, 3> ne_axis2;
  std::array<double, 3> direction;

  for (int i = 0; i < 100; i++) {
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne->ElongateTerminalEnd(10, direction);
    ne->RunDiscretization();
    scheduler.Simulate(1);

    ne_axis = ne->GetSpringAxis();
    EXPECT_GT(ne_axis[2], 0.1);
  }

  ne_axis = ne->GetSpringAxis();
  EXPECT_GT(ne->GetMassLocation()[2], 10);
  EXPECT_GT(ne_axis[2], 0.1);

  auto neList = ne->Bifurcate();
  auto ne2 = neList[1];
  ne = neList[0];

  for (int i = 0; i < 50; i++) {
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne->ElongateTerminalEnd(10, direction);
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne2->ElongateTerminalEnd(10, direction);
    ne->RunDiscretization();
    ne2->RunDiscretization();
    scheduler.Simulate(1);

    ne_axis = ne->GetSpringAxis();
    ne_axis2 = ne2->GetSpringAxis();
    EXPECT_GT(ne_axis[2], -0.5);
    EXPECT_GT(ne_axis2[2], 0.1);
  }

  EXPECT_GT(ne->GetMassLocation()[2], 15);
  EXPECT_GT(ne2->GetMassLocation()[2], 15);
}

TEST(MechanicalInteraction, TwoDistinctCylinderEncounter) {
  auto set_param = [](auto* param) {
    param->neurite_max_length_ = 2;
  };

  Simulation<> simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  gErrorIgnoreLevel = kWarning;

  auto neuron1 = rm->New<NeuronSoma>();
  neuron1.SetPosition({0, 0, 0});
  neuron1.SetMass(1);
  neuron1.SetDiameter(10);
  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({20, 0, 0});
  neuron2.SetMass(1);
  neuron2.SetDiameter(10);

  auto ne1 = neuron1.ExtendNewNeurite({0, 0, 1});
  ne1->SetDiameter(2);
  auto ne2 = neuron2.ExtendNewNeurite({0, 0, 1});
  ne2->SetDiameter(2);

  Scheduler<> scheduler;

  std::array<double, 3> direction1 = {0.5, 0, 1};
  std::array<double, 3> direction2 = {-0.5, 0, 1};

  std::array<double, 3> ne1_axis;
  std::array<double, 3> ne2_axis;

  for (int i = 0; i < 220; i++) {
    ne1->ElongateTerminalEnd(10, direction1);
    ne2->ElongateTerminalEnd(10, direction2);
    ne1->RunDiscretization();
    ne2->RunDiscretization();
    scheduler.Simulate(1);

    ne1_axis = ne1->GetSpringAxis();
    ne2_axis = ne2->GetSpringAxis();

    EXPECT_NEAR(ne1_axis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(ne2_axis[1], 0, abs_error<double>::value);
  }

  for (int i = 0; i < 100; i++) {
    ne1->ElongateTerminalEnd(10, direction1);
    ne2->ElongateTerminalEnd(10, direction2);
    ne1->RunDiscretization();
    ne2->RunDiscretization();
    scheduler.Simulate(1);

    ne1_axis = ne1->GetSpringAxis();
    ne2_axis = ne2->GetSpringAxis();

    EXPECT_NEAR(ne1_axis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(ne2_axis[1], 0, abs_error<double>::value);
  }
}

TEST(MechanicalInteraction, TwoCylinderGrowthObstacle) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  auto neuron1 = rm->New<NeuronSoma>();
  neuron1.SetPosition({0, 0, 0});
  neuron1.SetDiameter(6);

  auto neuron2 = rm->New<NeuronSoma>();
  neuron2.SetPosition({5.5, 0, 0});
  neuron2.SetDiameter(5);

  auto neuron3 = rm->New<NeuronSoma>();
  neuron3.SetPosition({6, 0, 16});
  neuron3.SetDiameter(10);

  auto ne1 = neuron1.ExtendNewNeurite({0, 0, 1});
  ne1->SetDiameter(1);
  auto ne2 = neuron2.ExtendNewNeurite({0, 0, 1});
  ne2->SetDiameter(1);

  Scheduler<> scheduler;

  std::array<double, 3> direction1 = {0.5, 0, 1};
  std::array<double, 3> direction2 = {0, 0, 1};

  std::array<double, 3> ne1_axis;
  std::array<double, 3> ne2_axis;

  std::array<double, 3> ne1_position;
  std::array<double, 3> ne2_position;

  for (int i = 0; i < 200; i++) {
    ne1->ElongateTerminalEnd(10, direction1);
    ne2->ElongateTerminalEnd(10, direction2);
    ne1->RunDiscretization();
    ne2->RunDiscretization();
    scheduler.Simulate(1);

    ne1_axis = ne1->GetSpringAxis();
    ne2_axis = ne2->GetSpringAxis();
    ne1_position = ne1->GetMassLocation();
    ne2_position = ne2->GetMassLocation();

    EXPECT_LT(ne1_position[0], ne2_position[0]);
  }

  ne1_axis = ne1->GetSpringAxis();
  ne2_axis = ne2->GetSpringAxis();
  ne1_position = ne1->GetMassLocation();
  ne2_position = ne2->GetMassLocation();

  EXPECT_NEAR(ne1_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne2_axis[1], 0, abs_error<double>::value);
  EXPECT_LT(ne1_position[0], ne2_position[0]);
  EXPECT_GT(ne1_position[2], 12);
  EXPECT_GT(ne2_position[2], 11);
}

}  // end namespace neuroscience
}  // end namespace experimental
}  // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
