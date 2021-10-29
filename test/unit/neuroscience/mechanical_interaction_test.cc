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

#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "gtest/gtest.h"
#include "neuroscience/module.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/param.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace neuroscience {
namespace mechanical_interaction_test_detail {

// -----------------------------------------------------------------------------
void RunStraightCylinderGrowthTest(const char* test_name,
                                   bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 0, 0})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  Double3 direction = {1, 0, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightxCylinderGrowth) {
  RunStraightCylinderGrowthTest(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightxCylinderGrowth_Static) {
  RunStraightCylinderGrowthTest(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest2(const char* test_name, bool detect_static_agents) {
  auto set_param = [&](bdm::Param* param) {
    param->unschedule_default_operations = {"mechanical forces"};
    param->Get<Param>()->neurite_max_length = 2;
    param->detect_static_agents = detect_static_agents;
  };
  neuroscience::InitModule();
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 0, 0})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 1, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  Double3 direction = {1, 0, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();
      double length = ne->GetActualLength();

      EXPECT_LT(length, 2.1);
      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightxCylinderGrowthNoMechanical) {
  RunTest2(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightxCylinderGrowthNoMechanical_Static) {
  RunTest2(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest3(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 1, 0})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);

  Double3 direction = {1, 1, 0};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], ne_axis[1], abs_error<double>::value);
      EXPECT_NEAR(ne_axis[2], 0, abs_error<double>::value);
    }
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalxyCylinderGrowth) {
  RunTest3(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalxyCylinderGrowth_Static) {
  RunTest3(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest4(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 1, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(1);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0.57735026918962584, abs_error<double>::value);

  Double3 direction = {1, 1, 1};
  for (int i = 0; i < 37; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);

    ne_axis = ne->GetSpringAxis();

    EXPECT_NEAR(ne_axis[0], ne_axis[1], abs_error<double>::value);
    EXPECT_NEAR(ne_axis[0], ne_axis[2], abs_error<double>::value);
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalxyzCylinderGrowth) {
  RunTest4(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalxyzCylinderGrowth_Static) {
  RunTest4(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest5(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 1, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0.57735026918962584, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 0.57735026918962584, abs_error<double>::value);

  Double3 direction = {2, 1, 1};

  for (int i = 0; i < 98; i++) {
    ne->ElongateTerminalEnd(300, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);

    ne_axis = ne->GetSpringAxis();

    EXPECT_TRUE(std::round(1e9 * ne_axis[1]) == std::round(1e9 * ne_axis[2]));
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalSpecialDirectionCylinderGrowth) {
  RunTest5(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, DiagonalSpecialDirectionCylinderGrowth_Static) {
  RunTest5(TEST_NAME, true);
}

// as the dendrite grows exactly at the center of the second cells
// growth force/direction and repulsive force/direction are equal
// so the dendrite stop growing
// -----------------------------------------------------------------------------
void RunTest6(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  NeuronSoma* neuron2 = new NeuronSoma();
  neuron2->SetPosition({0, 0, 30});
  neuron2->SetMass(1);
  neuron2->SetDiameter(10);
  rm->AddAgent(neuron2);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  simulation.GetExecutionContext()->SetupIterationAll(
      simulation.GetAllExecCtxts());

  Double3 direction = {0, 0, 1};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      ne_axis = ne->GetSpringAxis();

      EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
      EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
    }
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightCylinderGrowthObstacle) {
  RunTest6(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, StraightCylinderGrowthObstacle_Static) {
  RunTest6(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunNotStraightCylinderGrowthObstacleTest(const char* test_name,
                                              bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetDiameter(10);
  neuron->SetMass(1);
  rm->AddAgent(neuron);

  NeuronSoma* neuron2 = new NeuronSoma();
  neuron2->SetPosition({0, 0, 30});
  neuron2->SetDiameter(10);
  neuron2->SetMass(1);
  rm->AddAgent(neuron2);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  Double3 direction = {0.01, 0, 1};
  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);

    ne_axis = ne->GetSpringAxis();

    EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  }

  ne_axis = ne->GetSpringAxis();
  EXPECT_GT(ne->GetMassLocation()[0], 5);
  EXPECT_GT(ne_axis[0], 0);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, NotStraightCylinderGrowthObstacle) {
  RunNotStraightCylinderGrowthObstacleTest(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, NotStraightCylinderGrowthObstacle_Static) {
  RunNotStraightCylinderGrowthObstacleTest(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest7(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  Double3 direction = {0, 0.5, 1};
  Double3 direction2 = {0.5, 0, 1};

  for (int i = 0; i < 10; i++) {
    ne->ElongateTerminalEnd(100, {0, 0, 1});
    ne->RunDiscretization();
    scheduler->Simulate(1);
  }

  auto branches = ne->Bifurcate();
  auto branch_l = branches[0]->GetAgentPtr<NeuriteElement>();
  auto branch_r = branches[1]->GetAgentPtr<NeuriteElement>();

  for (int i = 0; i < 200; i++) {
    branch_r->ElongateTerminalEnd(100, direction);
    branch_r->RunDiscretization();
    branch_l->ElongateTerminalEnd(100, direction2);
    branch_l->RunDiscretization();
    scheduler->Simulate(1);
  }
  ne_axis = branch_l->GetSpringAxis();
  Double3 ne_axis_2 = branch_r->GetSpringAxis();

  EXPECT_NEAR(ne_axis[0], ne_axis_2[1], abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], ne_axis_2[0], abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], ne_axis_2[2], abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BifurcationCylinderGrowth) {
  RunTest7(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BifurcationCylinderGrowth_Static) {
  RunTest7(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest8(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis = ne->GetSpringAxis();

  Double3 direction = {0, 0.5, 1};
  Double3 direction2 = {0.5, 0, 1};

  for (int i = 0; i < 10; i++) {
    ne->ElongateTerminalEnd(100, {0, 0, 1});
    ne->RunDiscretization();
    scheduler->Simulate(1);
  }

  auto ne2 = ne->Branch(0.5, direction2)->GetAgentPtr<NeuriteElement>();

  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis[2], 1, abs_error<double>::value);

  for (int i = 0; i < 100; i++) {
    ne->ElongateTerminalEnd(100, direction);
    ne2->ElongateTerminalEnd(100, direction2);
    ne->RunDiscretization();
    ne2->RunDiscretization();

    scheduler->Simulate(1);
  }

  ne_axis = ne->GetSpringAxis();
  Double3 ne_axis_2 = ne2->GetSpringAxis();
  EXPECT_NEAR(ne_axis[0], 0, abs_error<double>::value);
  EXPECT_NEAR(ne_axis_2[1], 0, abs_error<double>::value);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BranchCylinderGrowth) {
  RunTest8(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BranchCylinderGrowth_Static) {
  RunTest8(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest9(const char* test_name, bool detect_static_agents) {
  auto set_param = [&](bdm::Param* param) {
    param->Get<Param>()->neurite_max_length = 2;
    param->detect_static_agents = detect_static_agents;
  };
  neuroscience::InitModule();
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* random = simulation.GetRandom();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();

  auto* scheduler = simulation.GetScheduler();

  Double3 ne_axis;
  Double3 ne_axis2;
  Double3 direction;

  for (int i = 0; i < 100; i++) {
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne->ElongateTerminalEnd(10, direction);
    ne->RunDiscretization();
    scheduler->Simulate(1);

    ne_axis = ne->GetSpringAxis();
    EXPECT_GT(ne_axis[2], 0.1);
  }

  ne_axis = ne->GetSpringAxis();
  EXPECT_GT(ne->GetMassLocation()[2], 10);
  EXPECT_GT(ne_axis[2], 0.1);

  auto ne_list = ne->Bifurcate();
  auto ne2 = ne_list[1]->GetAgentPtr<NeuriteElement>();
  ne = ne_list[0]->GetAgentPtr<NeuriteElement>();

  for (int i = 0; i < 50; i++) {
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne->ElongateTerminalEnd(10, direction);
    direction = {random->Uniform(-1, 1), random->Uniform(-1, 1), 1};
    ne2->ElongateTerminalEnd(10, direction);
    ne->RunDiscretization();
    ne2->RunDiscretization();
    scheduler->Simulate(1);

    ne_axis = ne->GetSpringAxis();
    ne_axis2 = ne2->GetSpringAxis();
    // cylinders split before being pushed away, so their daughters (two last
    // cylinders) attach point is pushed away as well while their terminal end
    // remain approximately at the same position, pulling them into a more
    // horizontal position
    EXPECT_GT(ne_axis[2], -0.5);
    EXPECT_GT(ne_axis2[2], 0);
  }

  EXPECT_GT(ne->GetMassLocation()[2], 15);
  EXPECT_GT(ne2->GetMassLocation()[2], 15);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BifurcateCylinderRandomGrowth) {
  RunTest9(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, BifurcateCylinderRandomGrowth_Static) {
  RunTest9(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest10(const char* test_name, bool detect_static_agents) {
  auto set_param = [&](bdm::Param* param) {
    param->Get<Param>()->neurite_max_length = 2;
    param->detect_static_agents = detect_static_agents;
  };

  neuroscience::InitModule();
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron1 = new NeuronSoma();
  neuron1->SetPosition({0, 0, 0});
  neuron1->SetDiameter(10);
  rm->AddAgent(neuron1);

  NeuronSoma* neuron2 = new NeuronSoma();
  neuron2->SetPosition({20, 0, 0});
  neuron2->SetDiameter(10);
  rm->AddAgent(neuron2);

  auto ne1 =
      neuron1->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne1->SetDiameter(2);
  auto ne2 =
      neuron2->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne2->SetDiameter(2);

  auto* scheduler = simulation.GetScheduler();

  Double3 direction1 = {0.5, 0, 1};
  Double3 direction2 = {-0.5, 0, 1};

  Double3 ne1_axis;
  Double3 ne2_axis;

  for (int i = 0; i < 220; i++) {
    ne1->ElongateTerminalEnd(10, direction1);
    ne2->ElongateTerminalEnd(10, direction2);
    ne1->RunDiscretization();
    ne2->RunDiscretization();
    scheduler->Simulate(1);

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
    scheduler->Simulate(1);

    ne1_axis = ne1->GetSpringAxis();
    ne2_axis = ne2->GetSpringAxis();

    EXPECT_NEAR(ne1_axis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(ne2_axis[1], 0, abs_error<double>::value);
  }
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, TwoDistinctCylinderEncounter) {
  RunTest10(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, TwoDistinctCylinderEncounter_Static) {
  RunTest10(TEST_NAME, true);
}

// -----------------------------------------------------------------------------
void RunTest11(const char* test_name, bool detect_static_agents) {
  neuroscience::InitModule();
  auto set_param = [&](bdm::Param* param) {
    param->detect_static_agents = detect_static_agents;
  };
  Simulation simulation(test_name, set_param);
  auto* rm = simulation.GetResourceManager();

  NeuronSoma* neuron1 = new NeuronSoma();
  neuron1->SetPosition({0, 0, 0});
  neuron1->SetDiameter(6);
  rm->AddAgent(neuron1);

  NeuronSoma* neuron2 = new NeuronSoma();
  neuron2->SetPosition({5.5, 0, 0});
  neuron2->SetDiameter(5);
  rm->AddAgent(neuron2);

  NeuronSoma* neuron3 = new NeuronSoma();
  neuron3->SetPosition({6, 0, 16});
  neuron3->SetDiameter(10);
  rm->AddAgent(neuron3);

  auto ne1 =
      neuron1->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne1->SetDiameter(1);
  auto ne2 =
      neuron2->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne2->SetDiameter(1);

  Double3 direction1 = {0.5, 0, 1};
  Double3 direction2 = {0, 0, 1};

  for (int i = 0; i < 200; i++) {
    ne1->ElongateTerminalEnd(10, direction1);
    ne2->ElongateTerminalEnd(10, direction2);
    ne1->RunDiscretization();
    ne2->RunDiscretization();
    simulation.Simulate(1);

    auto& ne1_position = ne1->GetMassLocation();
    auto& ne2_position = ne2->GetMassLocation();

    EXPECT_LT(ne1_position[0], ne2_position[0]);
  }

  auto& ne1_axis = ne1->GetSpringAxis();
  auto& ne2_axis = ne2->GetSpringAxis();
  auto& ne1_position = ne1->GetMassLocation();
  auto& ne2_position = ne2->GetMassLocation();

  EXPECT_NEAR(ne1_axis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(ne2_axis[1], 0, abs_error<double>::value);
  EXPECT_LT(ne1_position[0], ne2_position[0]);
  EXPECT_GT(ne1_position[2], 12);
  EXPECT_GT(ne2_position[2], 11);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, TwoCylinderGrowthObstacle) {
  RunTest11(TEST_NAME, false);
}

// -----------------------------------------------------------------------------
TEST(MechanicalInteraction, TwoCylinderGrowthObstacle_Static) {
  RunTest11(TEST_NAME, true);
}

}  // end namespace mechanical_interaction_test_detail
}  // end namespace neuroscience
}  // end namespace bdm
