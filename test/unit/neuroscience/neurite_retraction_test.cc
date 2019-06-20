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
namespace experimental {
namespace neuroscience {

TEST(NeuriteElementBehaviour, StraightxCylinderGrowthRetract) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();

  NeuronSoma* neuron = new NeuronSoma();
  auto neuron_id = neuron->GetUid();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->push_back(neuron);

  auto* ne = dynamic_cast<NeuronSoma*>(rm->GetSimObject(neuron_id))
                 ->ExtendNewNeurite({1, 0, 0});

  Double3 neAxis = ne->GetSpringAxis();

  EXPECT_NEAR(neAxis[0], 1, abs_error<double>::value);
  EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
  EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

  for (int i = 0; i < 50; i++) {
    ne->ElongateTerminalEnd(100, {1, 0, 0});
    ne->RunDiscretization();
    scheduler->Simulate(1);
    if (i % 10 == 0) {
      neAxis = ne->GetSpringAxis();
      EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
    }
  }

  // while there are still neurite elements left
  while (rm->GetNumSimObjects() != 1) {
    ne->RetractTerminalEnd(50);
    scheduler->Simulate(1);

    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
  }
}

TEST(NeuriteElementBehaviour, BranchingGrowth) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* random = simulation.GetRandom();

  double branching_factor = 0.005;

  NeuronSoma* neuron = new NeuronSoma();
  auto neuron_id = neuron->GetUid();
  neuron->SetPosition({0, 0, 0});
  neuron->SetMass(1);
  neuron->SetDiameter(10);
  rm->push_back(neuron);

  auto* ne = dynamic_cast<NeuronSoma*>(rm->GetSimObject(neuron_id))
                 ->ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(1);

  Double3 previous_direction;
  Double3 direction;

  for (int i = 0; i < 200; i++) {
    rm->ApplyOnAllElements([&](SimObject* so) {
      if (auto* ne = dynamic_cast<NeuriteElement*>(so)) {
        EXPECT_GT(ne->GetAxis()[2], 0);

        if (ne->IsTerminal() && ne->GetDiameter() > 0.5) {
          previous_direction = ne->GetSpringAxis();
          direction = {random->Uniform(-10, 10), random->Uniform(-10, 10),
                       random->Uniform(0, 5)};

          Double3 step_direction = previous_direction + direction;

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
  while (rm->GetNumSimObjects() != 1) {
    rm->ApplyOnAllElements([&](SimObject* so) {
      if (auto* ne = dynamic_cast<NeuriteElement*>(so)) {
        ne->RetractTerminalEnd(50);
      }
    });
    scheduler->Simulate(1);
  }
  EXPECT_EQ(1u, rm->GetNumSimObjects());
}  // end test

}  // end namespace neuroscience
}  // end namespace experimental
}  // end namespace bdm
