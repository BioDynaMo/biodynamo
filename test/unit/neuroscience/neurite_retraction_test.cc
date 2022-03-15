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

TEST(NeuriteElementBehaviour, StraightxCylinderGrowthRetract) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({1, 0, 0})->GetAgentPtr<NeuriteElement>();

  Real3 neAxis = ne->GetSpringAxis();

  EXPECT_NEAR(neAxis[0], 1, abs_error<real>::value);
  EXPECT_NEAR(neAxis[1], 0, abs_error<real>::value);
  EXPECT_NEAR(neAxis[2], 0, abs_error<real>::value);

  for (int i = 0; i < 50; i++) {
    ne->ElongateTerminalEnd(100, {1, 0, 0});
    ne->RunDiscretization();
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      neAxis = ne->GetSpringAxis();
      EXPECT_NEAR(neAxis[1], 0, abs_error<real>::value);
      EXPECT_NEAR(neAxis[2], 0, abs_error<real>::value);
    }
  }

  // while there are still neurite elements left
  while (rm->GetNumAgents() != 1) {
    ne->RetractTerminalEnd(50);
    scheduler->Simulate(1);

    EXPECT_NEAR(neAxis[1], 0, abs_error<real>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<real>::value);
  }
}

TEST(NeuriteElementBehaviour, BranchingGrowth) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* random = simulation.GetRandom();

  real branching_factor = 0.005;

  NeuronSoma* neuron = new NeuronSoma();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->AddAgent(neuron);

  auto ne = neuron->ExtendNewNeurite({0, 0, 1})->GetAgentPtr<NeuriteElement>();
  ne->SetDiameter(1);

  Real3 previous_direction;
  Real3 direction;

  for (int i = 0; i < 200; i++) {
    rm->ForEachAgent([&](Agent* agent) {
      if (auto* ne = dynamic_cast<NeuriteElement*>(agent)) {
        EXPECT_GT(ne->GetAxis()[2], 0);

        if (ne->IsTerminal() && ne->GetDiameter() > 0.5) {
          previous_direction = ne->GetSpringAxis();
          direction = {random->Uniform(-10, 10), random->Uniform(-10, 10),
                       random->Uniform(0, 5)};

          Real3 step_direction = previous_direction + direction;

          ne->ElongateTerminalEnd(10, step_direction);
          ne->SetDiameter(1);

          if (random->Uniform(0, 1) < branching_factor * ne->GetDiameter()) {
            ne->Bifurcate();
          }
        }
      }
    });
    scheduler->Simulate(1);
  }

  // while there are still neurite elements left
  while (rm->GetNumAgents() != 1) {
    rm->ForEachAgent([&](Agent* agent) {
      if (auto* ne = dynamic_cast<NeuriteElement*>(agent)) {
        ne->RetractTerminalEnd(50);
      }
    });
    scheduler->Simulate(1);
  }
  EXPECT_EQ(1u, rm->GetNumAgents());
}  // end test

}  // end namespace neuroscience
}  // end namespace bdm
