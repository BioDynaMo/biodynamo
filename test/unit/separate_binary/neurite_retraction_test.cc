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

#include "biodynamo.h"
#include "core/sim_object/backend.h"
#include "core/sim_object/cell.h"
#include "core/simulation_implementation.h"
#include "gtest/gtest.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/param.h"
#include "unit/test_util/test_util.h"

namespace bdm {

BDM_CTPARAM(experimental::neuroscience) {
  BDM_CTPARAM_HEADER(experimental::neuroscience);

  using SimObjectTypes = CTList<Cell, experimental::neuroscience::NeuronSoma,
                                experimental::neuroscience::NeuriteElement>;
};

namespace experimental {
namespace neuroscience {

TEST(NeuriteElementBehaviour, StraightxCylinderGrowthRetract) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();

  NeuronSoma neuron;
  auto neuron_id = neuron.GetUid();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);
  rm->push_back(neuron);

  auto ne = rm->GetSimObject<NeuronSoma>(neuron_id).ExtendNewNeurite({1, 0, 0});

  std::array<double, 3> neAxis = ne->GetSpringAxis();

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

  while (rm->Get<NeuriteElement>()->size() != 0) {
    ne->RetractTerminalEnd(50);
    scheduler->Simulate(1);

    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
  }
}

TEST(NeuriteElementBehaviour, BranchingGrowth) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* random = simulation.GetRandom();

  double branching_factor = 0.005;

  NeuronSoma neuron;
  auto neuron_id = neuron.GetUid();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);
  rm->push_back(neuron);

  auto ne = rm->GetSimObject<NeuronSoma>(neuron_id).ExtendNewNeurite({0, 0, 1});
  ne->SetDiameter(1);

  std::array<double, 3> previous_direction;
  std::array<double, 3> direction;

  for (int i = 0; i < 200; i++) {
    const auto* my_neurites = rm->Get<NeuriteElement>();
    int num_neurites = my_neurites->size();

    // for each neurite in simulation
    for (int neurite_nb = 0; neurite_nb < num_neurites; neurite_nb++) {
      auto ne = (*my_neurites)[neurite_nb].GetSoPtr();

      EXPECT_GT(ne->GetAxis()[2], 0);

      if (ne->IsTerminal() && ne->GetDiameter() > 0.5) {
        previous_direction = ne->GetSpringAxis();
        direction = {random->Uniform(-10, 10), random->Uniform(-10, 10),
                     random->Uniform(0, 5)};

        std::array<double, 3> step_direction =
            Math::Add(previous_direction, direction);

        ne->ElongateTerminalEnd(10, step_direction);
        ne->SetDiameter(1);

        if (random->Uniform(0, 1) < branching_factor * ne->GetDiameter()) {
          ne->Bifurcate();
        }
      }
    }
    scheduler->Simulate(1);
  }

  while (rm->Get<NeuriteElement>()->size() != 0) {
    auto my_neurites = rm->Get<NeuriteElement>();
    int num_neurites = my_neurites->size();

    // for each neurite in simulation
    for (int neurite_nb = 0; neurite_nb < num_neurites; neurite_nb++) {
      auto ne = (*my_neurites)[neurite_nb].GetSoPtr();
      ne->RetractTerminalEnd(50);
    }
    scheduler->Simulate(1);
  }
  EXPECT_NEAR(rm->Get<NeuriteElement>()->size(), 0, abs_error<double>::value);
}  // end test

}  // end namespace neuroscience
}  // end namespace experimental
}  // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
