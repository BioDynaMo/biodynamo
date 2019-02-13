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

#include "neuroscience/neuron_soma.h"
#include "gtest/gtest.h"

#include "core/resource_manager.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/module.h"
#include "neuroscience/param.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

TEST(NeuronSomaTest, ExtendNewNeuriteElementSphericalCoordinates) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  ctxt->SetupIteration();

  // create neuron
  std::array<double, 3> origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->push_back(neuron);

  // new neurite
  auto neurite = rm->GetSimObject(neuron_id)->As<NeuronSoma>()->ExtendNewNeurite(
      10, Math::kPi / 8, Math::kPi / 3);
  neurite->SetDiameter(2);

  ctxt->TearDownIteration();

  // verify
  EXPECT_ARR_NEAR(neurite->GetPosition(),
                  {8.4010830245082868, 3.4798425273737141, 5.2500000000000009});
  EXPECT_ARR_NEAR(neurite->GetMassLocation(),
                  {8.8011345971039194, 3.6455493143915101, 5.5});
  EXPECT_ARR_NEAR(neurite->GetXAxis(),
                  {0.80010314519126546, 0.3314135740355918, 0.5});
  EXPECT_ARR_NEAR(
      neurite->GetYAxis(),
      {-0.28104832556833692, 0.94348558173665553, -0.17563255891285873});
  EXPECT_ARR_NEAR(neurite->GetZAxis(),
                  {-0.52994980493465504, 0, 0.84802901144343001});
  EXPECT_ARR_NEAR(neurite->GetSpringAxis(),
                  {0.80010314519126546, 0.3314135740355918, 0.5});
  EXPECT_NEAR(3.1415926535897931, neurite->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, neurite->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, neurite->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(neurite->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(neurite->GetDaughterRight() == nullptr);
  EXPECT_TRUE(neurite->GetMother()->As<NeuronSoma>() != nullptr);

  EXPECT_EQ(2u, rm->GetNumSimObjects());
}

TEST(NeuronSomaTest, ExtendNewNeurite) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  ctxt->SetupIteration();

  // create neuron
  std::array<double, 3> origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->push_back(neuron);

  // new neurite
  auto neurite = rm->GetSimObject(neuron_id)->As<NeuronSoma>()->ExtendNewNeurite({0, 0, 1});
  neurite->SetDiameter(2);

  ctxt->TearDownIteration();

  // verify
  EXPECT_ARR_NEAR(neurite->GetPosition(), {0, 0, 10.5});
  EXPECT_ARR_NEAR(neurite->GetMassLocation(), {0, 0, 11});
  EXPECT_ARR_NEAR(neurite->GetXAxis(), {0, 0, 1});
  EXPECT_ARR_NEAR(neurite->GetYAxis(), {0, 1, 0});
  EXPECT_ARR_NEAR(neurite->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(neurite->GetSpringAxis(), {0, 0, 1});
  EXPECT_NEAR(3.1415926535897931, neurite->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, neurite->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, neurite->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(neurite->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(neurite->GetDaughterRight() == nullptr);
  EXPECT_TRUE(neurite->GetMother()->As<NeuronSoma>() != nullptr);

  EXPECT_EQ(2u, rm->GetNumSimObjects());
}

TEST(NeuronSomaTest, ExtendNeuriteAndElongate) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  std::array<double, 3> origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->push_back(neuron);

  auto neurite_element =
      rm->GetSimObject(neuron_id)->As<NeuronSoma>()->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  ctxt->TearDownIteration();

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0, 1});
    neurite_element->RunDiscretization();
  }

  ctxt->TearDownIteration();

  // verify
  //   distal segment
  EXPECT_ARR_NEAR(neurite_element->GetMassLocation(), {0, 0, 31});
  EXPECT_ARR_NEAR(neurite_element->GetPosition(), {0, 0, 27.25});
  EXPECT_ARR_NEAR(neurite_element->GetXAxis(), {0, 0, 1});
  EXPECT_ARR_NEAR(neurite_element->GetYAxis(), {0, 1, 0});
  EXPECT_ARR_NEAR(neurite_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(neurite_element->GetSpringAxis(), {0, 0, 7.5});
  EXPECT_NEAR(23.561944901923749, neurite_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(7.5, neurite_element->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(7.5, neurite_element->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(neurite_element->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(neurite_element->GetDaughterRight() == nullptr);
  EXPECT_TRUE(neurite_element->GetMother()->As<NeuriteElement>() != nullptr);

  //   proximal segment
  auto proximal_element = neurite_element->GetMother()->As<NeuriteElement>();
  EXPECT_ARR_NEAR(proximal_element->GetMassLocation(), {0, 0, 23.5});
  EXPECT_ARR_NEAR(proximal_element->GetPosition(), {0, 0, 16.75});
  EXPECT_ARR_NEAR(proximal_element->GetXAxis(), {0, 0, 1});
  EXPECT_ARR_NEAR(proximal_element->GetYAxis(), {0, 1, 0});
  EXPECT_ARR_NEAR(proximal_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(proximal_element->GetSpringAxis(), {0, 0, 13.5});
  EXPECT_NEAR(42.411500823462518, proximal_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, proximal_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, proximal_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(13.5, proximal_element->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, proximal_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, proximal_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(13.5, proximal_element->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(proximal_element->GetDaughterLeft() != nullptr);
  EXPECT_TRUE(proximal_element->GetDaughterRight() == nullptr);
  EXPECT_TRUE(proximal_element->GetMother()->As<NeuronSoma>() != nullptr);

  EXPECT_EQ(3u, rm->GetNumSimObjects());
}

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
