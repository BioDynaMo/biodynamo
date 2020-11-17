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
#include "neuroscience/module.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/param.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace neuroscience {

TEST(NeuronSomaTest, ExtendNewNeuriteElementSphericalCoordinates) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  ctxt->SetupIterationAll(simulation.GetAllExecCtxts());

  // create neuron
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  // new neurite
  auto neurite = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                     ->ExtendNewNeurite(10, Math::kPi / 8, Math::kPi / 3);
  neurite->SetDiameter(2);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());

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
  EXPECT_TRUE(dynamic_cast<NeuronSoma*>(neurite->GetMother().Get()) != nullptr);

  EXPECT_EQ(2u, rm->GetNumAgents());
}

TEST(NeuronSomaTest, ExtendNewNeurite) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  ctxt->SetupIterationAll(simulation.GetAllExecCtxts());

  // create neuron
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  // new neurite
  auto neurite = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                     ->ExtendNewNeurite({0, 0, 1});
  neurite->SetDiameter(2);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());

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
  EXPECT_TRUE(dynamic_cast<NeuronSoma*>(neurite->GetMother().Get()) != nullptr);

  EXPECT_EQ(2u, rm->GetNumAgents());
}

TEST(NeuronSomaTest, ExtendNeuriteAndElongate) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0, 1});
    neurite_element->RunDiscretization();
  }

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());

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
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(
                  neurite_element->GetMother().Get()) != nullptr);

  //   proximal segment
  auto proximal_element =
      dynamic_cast<NeuriteElement*>(neurite_element->GetMother().Get());
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
  EXPECT_TRUE(dynamic_cast<NeuronSoma*>(proximal_element->GetMother().Get()) !=
              nullptr);

  EXPECT_EQ(3u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, PartialRetraction) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0, 1});
    neurite_element->RunDiscretization();
  }

  // will remove the proximal segment
  for (int i = 0; i < 140; ++i) {
    ctxt->SetupIterationAll(simulation.GetAllExecCtxts());
    neurite_element->RetractTerminalEnd(10);
    neurite_element->RunDiscretization();
    ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  }

  // verify
  EXPECT_ARR_NEAR(neurite_element->GetMassLocation(), {0, 0, 17});
  EXPECT_ARR_NEAR(neurite_element->GetPosition(), {0, 0, 13.5});
  EXPECT_ARR_NEAR(neurite_element->GetXAxis(), {0, 0, 1});
  EXPECT_ARR_NEAR(neurite_element->GetYAxis(), {0, 1, 0});
  EXPECT_ARR_NEAR(neurite_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(neurite_element->GetSpringAxis(), {0, 0, 7});
  EXPECT_NEAR(21.991148575129266, neurite_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(7, neurite_element->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(7, neurite_element->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(neurite_element->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(neurite_element->GetDaughterRight() == nullptr);
  EXPECT_TRUE(dynamic_cast<NeuronSoma*>(neurite_element->GetMother().Get()) !=
              nullptr);

  EXPECT_EQ(2u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, TotalRetraction) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0, 1});
    neurite_element->RunDiscretization();
  }

  // will remove the entire neurite
  // neurite_segment will be removed in iteration 209
  for (int i = 0; i < 210; ++i) {
    ctxt->SetupIterationAll(simulation.GetAllExecCtxts());
    neurite_element->RetractTerminalEnd(10);
    neurite_element->RunDiscretization();
    ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  }

  // verify
  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_EQ(0u, neuron->GetDaughters().size());
}

TEST(NeuriteElementTest, Branch) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0.5, 0.5});
    neurite_element->RunDiscretization();
  }

  auto* branch = neurite_element->Branch({0, 1, 0});

  // verify
  //  neurite segment
  EXPECT_ARR_NEAR(neurite_element->GetMassLocation(),
                  {0, 14.142135623730928, 25.142135623730923});
  EXPECT_ARR_NEAR(neurite_element->GetPosition(),
                  {0, 12.881717786265909, 23.856717786265904});
  EXPECT_ARR_NEAR(neurite_element->GetXAxis(),
                  {0, 0.70012926565611089, 0.7140161142242063});
  EXPECT_ARR_NEAR(neurite_element->GetYAxis(),
                  {0, 0.71401611422420619, -0.70012926565611078});
  EXPECT_ARR_NEAR(neurite_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(neurite_element->GetSpringAxis(),
                  {0, 2.5208356749300371, 2.5708356749300378});
  EXPECT_NEAR(11.311395231915842, neurite_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(3.6005289288510043, neurite_element->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(3.6005289288510043, neurite_element->GetRestingLength(),
              kEpsilon);
  EXPECT_TRUE(neurite_element->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(neurite_element->GetDaughterRight() == nullptr);
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(
                  neurite_element->GetMother().Get()) != nullptr);

  //  proximal segment
  auto proximal_element =
      dynamic_cast<NeuriteElement*>(neurite_element->GetMother().Get());
  EXPECT_ARR_NEAR(proximal_element->GetMassLocation(),
                  {0, 11.621299948800891, 22.571299948800885});
  EXPECT_ARR_NEAR(proximal_element->GetPosition(),
                  {0, 10.360882111335872, 21.285882111335866});
  EXPECT_ARR_NEAR(proximal_element->GetXAxis(),
                  {0, 0.70012926565611089, 0.7140161142242063});
  EXPECT_ARR_NEAR(proximal_element->GetYAxis(),
                  {0, 0.71401611422420619, -0.70012926565611078});
  EXPECT_ARR_NEAR(proximal_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(proximal_element->GetSpringAxis(),
                  {0, 2.5208356749300371, 2.5708356749300378});
  EXPECT_NEAR(11.311395231915842, proximal_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, proximal_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, proximal_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(3.6005289288510043, proximal_element->GetActualLength(),
              kEpsilon);
  EXPECT_NEAR(0, proximal_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, proximal_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(3.6005289288510043, proximal_element->GetRestingLength(),
              kEpsilon);
  EXPECT_TRUE(proximal_element->GetDaughterLeft() != nullptr);
  EXPECT_TRUE(proximal_element->GetDaughterRight() != nullptr);
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(
                  proximal_element->GetMother().Get()) != nullptr);

  //  new branch
  EXPECT_ARR_NEAR(branch->GetPosition(),
                  {0, 12.121299948800891, 22.571299948800885});
  EXPECT_ARR_NEAR(branch->GetXAxis(), {0, 1, 0});
  EXPECT_ARR_NEAR(branch->GetYAxis(), {0, 0, -1});
  EXPECT_ARR_NEAR(branch->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(branch->GetSpringAxis(), {0, 1, 0});
  EXPECT_NEAR(3.1415926535897931, branch->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, branch->GetDiameter(), kEpsilon);
  EXPECT_NEAR(1, branch->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, branch->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, branch->GetTension(), kEpsilon);
  EXPECT_NEAR(10, branch->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, branch->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(branch->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(branch->GetDaughterRight() == nullptr);
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(branch->GetMother().Get()) !=
              nullptr);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(5u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, RightDaughterRetraction) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0.5, 0.5});
    neurite_element->RunDiscretization();
  }

  auto* branch = neurite_element->Branch({0, 1, 0});

  for (int i = 0; i < 100; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, -0.5, 1});
    neurite_element->RunDiscretization();
    branch->ElongateTerminalEnd(10, {0, 1, 0.5});
    branch->RunDiscretization();
  }

  EXPECT_NEAR(11.6792669065954, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(10.9036023322569, branch->GetLength(), kEpsilon);

  auto* proximal_element =
      dynamic_cast<NeuriteElement*>(neurite_element->GetMother().Get());
  auto right_daughter_pe = proximal_element->GetDaughterRight();
  for (int i = 0; i < 40; ++i) {
    right_daughter_pe->RetractTerminalEnd(10);
    right_daughter_pe->RunDiscretization();
  }

  // verify
  EXPECT_NEAR(11.6792669065954, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(6.90360233225694, branch->GetLength(), kEpsilon);

  //  new branch
  EXPECT_ARR_NEAR(branch->GetMassLocation(),
                  {0, 17.9175034106028, 25.4028272980485});
  EXPECT_ARR_NEAR(branch->GetPosition(),
                  {0, 14.769401679701861, 23.987063623424685});
  EXPECT_ARR_NEAR(branch->GetXAxis(),
                  {0, 0.912017112049318, 0.410152151438001});
  EXPECT_ARR_NEAR(branch->GetYAxis(),
                  {0, 0.410152151438001, -0.912017112049318});
  EXPECT_ARR_NEAR(branch->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(branch->GetSpringAxis(),
                  {0, 6.29620346180194, 2.8315273492476});
  EXPECT_NEAR(21.688306370323865, branch->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, branch->GetDiameter(), kEpsilon);
  EXPECT_NEAR(1, branch->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(6.90360233225697, branch->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, branch->GetTension(), kEpsilon);
  EXPECT_NEAR(10, branch->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(6.90360233225697, branch->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(branch->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(branch->GetDaughterRight() == nullptr);
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(branch->GetMother().Get()) !=
              nullptr);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(5u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, RightDaughterTotalRetraction) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0.5, 0.5});
    neurite_element->RunDiscretization();
  }

  auto* branch = neurite_element->Branch({0, 1, 0});

  for (int i = 0; i < 100; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, -0.5, 1});
    neurite_element->RunDiscretization();
    branch->ElongateTerminalEnd(10, {0, 1, 0.5});
    branch->RunDiscretization();
  }

  EXPECT_NEAR(11.6792669065954, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(10.9036023322569, branch->GetLength(), kEpsilon);

  auto* proximal_element =
      dynamic_cast<NeuriteElement*>(neurite_element->GetMother().Get());
  auto right_daughter_pe = proximal_element->GetDaughterRight();
  // right_daughter_ps == branch
  while (proximal_element->GetDaughterRight() != nullptr) {
    right_daughter_pe->RetractTerminalEnd(10);
    right_daughter_pe->RunDiscretization();
  }

  // verify
  EXPECT_NEAR(11.6792669065954, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(0.103602332256979, branch->GetLength(), kEpsilon);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(4u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, LeftDaughterRetraction) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                             ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {0, 0.5, 0.5});
    neurite_element->RunDiscretization();
  }

  auto* branch = neurite_element->Branch({0, 1, 0});

  for (int i = 0; i < 100; ++i) {
    neurite_element->ElongateTerminalEnd(10, {-0.5, 0.5, 1});
    neurite_element->RunDiscretization();
    branch->ElongateTerminalEnd(10, {0, 1, 0.5});
    branch->RunDiscretization();
  }

  EXPECT_NEAR(13.2486948956586, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(10.903602332257, branch->GetLength(), kEpsilon);

  auto* proximal_element =
      dynamic_cast<NeuriteElement*>(neurite_element->GetMother().Get());
  auto left_daughter_pe = proximal_element->GetDaughterLeft();
  for (int i = 0; i < 10; ++i) {
    left_daughter_pe->RetractTerminalEnd(10);
    left_daughter_pe->RunDiscretization();
  }

  // verify
  EXPECT_NEAR(12.2486948956586, neurite_element->GetLength(), kEpsilon);
  EXPECT_NEAR(10.903602332257, branch->GetLength(), kEpsilon);

  //  new branch
  EXPECT_ARR_NEAR(branch->GetMassLocation(),
                  {0, 21.5655718588001, 27.0434359038005});
  EXPECT_ARR_NEAR(branch->GetPosition(),
                  {0, 16.59343590380049, 24.807367926300685});
  EXPECT_ARR_NEAR(branch->GetXAxis(),
                  {0, 0.912017112049318, 0.410152151438001});
  EXPECT_ARR_NEAR(branch->GetYAxis(),
                  {0, 0.410152151438001, -0.912017112049318});
  EXPECT_ARR_NEAR(branch->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(branch->GetSpringAxis(),
                  {0, 9.9442719099992, 4.4721359549996});
  EXPECT_NEAR(34.254676984682995, branch->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, branch->GetDiameter(), kEpsilon);
  EXPECT_NEAR(1, branch->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(10.903602332257, branch->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, branch->GetTension(), kEpsilon);
  EXPECT_NEAR(10, branch->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(10.903602332257, branch->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(branch->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(branch->GetDaughterRight() == nullptr);
  EXPECT_TRUE(dynamic_cast<NeuriteElement*>(branch->GetMother().Get()) !=
              nullptr);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(5u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, RetractAllDendrites) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto* neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                              ->ExtendNewNeurite({1, 0, 0});
  neurite_element->SetDiameter(2);

  // will create a new neurite segment at iteration 139
  for (int i = 0; i < 200; ++i) {
    neurite_element->ElongateTerminalEnd(10, {1, 1, 0});
    neurite_element->RunDiscretization();
    ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  }

  auto* branch = neurite_element->Branch();

  for (int i = 0; i < 100; ++i) {
    ctxt->SetupIterationAll(simulation.GetAllExecCtxts());
    neurite_element->ElongateTerminalEnd(10, {0, 0, 1});
    neurite_element->RunDiscretization();
    branch->ElongateTerminalEnd(10, {0, 1, 0});
    branch->RunDiscretization();
    ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  }

  // retract all dendrite
  while (rm->GetNumAgents() != 1) {
    rm->ForEachAgent([&](Agent* agent) {
      if (auto* neurite_segment = dynamic_cast<NeuriteElement*>(agent)) {
        if (neurite_segment->IsTerminal()) {
          neurite_segment->RetractTerminalEnd(10);
          neurite_segment->RunDiscretization();
        }
      }
    });
    ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  }

  // verify
  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(1u, rm->GetNumAgents());
}

TEST(NeuriteElementTest, Bifurcate) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();

  const double kEpsilon = abs_error<double>::value;
  Double3 origin = {0, 0, 0};
  NeuronSoma* neuron = new NeuronSoma(origin);
  neuron->SetDiameter(20);
  auto neuron_id = neuron->GetUid();
  rm->AddAgent(neuron);

  auto* neurite_element = dynamic_cast<NeuronSoma*>(rm->GetAgent(neuron_id))
                              ->ExtendNewNeurite({0, 0, 1});
  neurite_element->SetDiameter(2);

  auto bifurcation = neurite_element->Bifurcate({0, 1, 1}, {1, 1, 0});

  // verify
  //  neurite segment
  EXPECT_ARR_NEAR(neurite_element->GetMassLocation(), {0, 0, 11});
  EXPECT_ARR_NEAR(neurite_element->GetPosition(), {0, 0, 10.5});
  EXPECT_ARR_NEAR(neurite_element->GetXAxis(), {0, 0, 1});
  EXPECT_ARR_NEAR(neurite_element->GetYAxis(), {0, 1, -0});
  EXPECT_ARR_NEAR(neurite_element->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(neurite_element->GetSpringAxis(), {0, 0, 1});
  EXPECT_NEAR(3.1415926535897931, neurite_element->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, neurite_element->GetDiameter(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, neurite_element->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, neurite_element->GetTension(), kEpsilon);
  EXPECT_NEAR(10, neurite_element->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, neurite_element->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(neurite_element->GetDaughterLeft() != nullptr);
  EXPECT_TRUE(neurite_element->GetDaughterRight() != nullptr);
  EXPECT_TRUE(neurite_element->GetMother()->IsNeuronSoma());

  //  left branch
  auto* branch_l = bifurcation[0];
  EXPECT_ARR_NEAR(branch_l->GetMassLocation(),
                  {0, 0.707106781186547, 11.7071067811865});
  EXPECT_ARR_NEAR(branch_l->GetPosition(),
                  {0, 0.353553390593274, 11.3535533905933});
  EXPECT_ARR_NEAR(branch_l->GetXAxis(),
                  {0, 0.707106781186548, 0.707106781186548});
  EXPECT_ARR_NEAR(branch_l->GetYAxis(),
                  {0, 0.707106781186548, -0.707106781186548});
  EXPECT_ARR_NEAR(branch_l->GetZAxis(), {-1, 0, 0});
  EXPECT_ARR_NEAR(branch_l->GetSpringAxis(),
                  {0, 0.707106781186547, 0.707106781186548});
  EXPECT_NEAR(3.1415926535897931, branch_l->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, branch_l->GetDiameter(), kEpsilon);
  EXPECT_NEAR(1, branch_l->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, branch_l->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, branch_l->GetTension(), kEpsilon);
  EXPECT_NEAR(10, branch_l->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, branch_l->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(branch_l->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(branch_l->GetDaughterRight() == nullptr);
  EXPECT_TRUE(branch_l->GetMother()->IsNeuriteElement());

  //  right branch
  auto* branch_r = bifurcation[1];
  EXPECT_ARR_NEAR(branch_r->GetMassLocation(),
                  {0.707106781186547, 0.707106781186547, 11});
  EXPECT_ARR_NEAR(branch_r->GetPosition(),
                  {0.353553390593274, 0.353553390593274, 11});
  EXPECT_ARR_NEAR(branch_r->GetXAxis(),
                  {0.707106781186548, 0.707106781186548, 0});
  EXPECT_ARR_NEAR(branch_r->GetYAxis(),
                  {-0.707106781186548, 0.707106781186548, -0});
  EXPECT_ARR_NEAR(branch_r->GetZAxis(), {-0, 0, 1});
  EXPECT_ARR_NEAR(branch_r->GetSpringAxis(),
                  {0.707106781186547, 0.707106781186547, 0});
  EXPECT_NEAR(3.1415926535897931, branch_r->GetVolume(), kEpsilon);
  EXPECT_NEAR(2, branch_r->GetDiameter(), kEpsilon);
  EXPECT_NEAR(1, branch_r->GetBranchOrder(), kEpsilon);
  EXPECT_NEAR(1, branch_r->GetActualLength(), kEpsilon);
  EXPECT_NEAR(0, branch_r->GetTension(), kEpsilon);
  EXPECT_NEAR(10, branch_r->GetSpringConstant(), kEpsilon);
  EXPECT_NEAR(1, branch_r->GetRestingLength(), kEpsilon);
  EXPECT_TRUE(branch_r->GetDaughterLeft() == nullptr);
  EXPECT_TRUE(branch_r->GetDaughterRight() == nullptr);
  EXPECT_TRUE(branch_r->GetMother()->IsNeuriteElement());

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());
  EXPECT_EQ(4u, rm->GetNumAgents());
}

TEST(DISABLED_NeuronSomaNeuriteElementTest, Displacement) {
  // Simulation simulation(TEST_NAME);
  // auto* rm = simulation.GetResourceManager();
  //
  // auto* neurons = rm->template Get<NeuronSoma>();
  // auto* neurite_segments = rm->template Get<NeuriteElement>();
  //
  // Cell 1
  // auto&& cell1 = rm->template New<Cell>();
  // cell1.SetAdherence(0.3);
  // cell1.SetDiameter(9);
  // cell1.SetMass(1.4);
  // cell1.SetPosition({0, 0, 0});
  //
  // // Cell 2
  // auto&& cell2 = rm->template New<Cell>();
  // cell2.SetAdherence(0.4);
  // cell2.SetDiameter(11);
  // cell2.SetMass(1.1);
  // cell2.SetPosition({0, 5, 0});
  //
  // simulation.GetEnvironment()->Initialize();
  //
  // execute operation
  // MechanicalForcesOp<> op;
  // op(neurons, 0);
  // op(neurite_segments, 1);
  //
  // // check results
  // // cell 1
  // auto final_position = (*cells)[0].GetPosition();
  // EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  // EXPECT_NEAR(-0.07797206232558615, final_position[1],
  //             abs_error<double>::value);
  // EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // // cell 2
  // final_position = (*cells)[1].GetPosition();
  // EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  // EXPECT_NEAR(5.0992371702325645, final_position[1],
  // abs_error<double>::value);
  // EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  //
  // // check if tractor_force has been reset to zero
  // // cell 1
  // auto final_tf = (*cells)[0].GetTractorForce();
  // EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  // EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  // EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // // cell 2
  // final_tf = (*cells)[1].GetTractorForce();
  // EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  // EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  // EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  //
  // // remaining fields should remain unchanged
  // // cell 1
  // EXPECT_NEAR(0.3, (*cells)[0].GetAdherence(), abs_error<double>::value);
  // EXPECT_NEAR(9, (*cells)[0].GetDiameter(), abs_error<double>::value);
  // EXPECT_NEAR(1.4, (*cells)[0].GetMass(), abs_error<double>::value);
  // // cell 2
  // EXPECT_NEAR(0.4, (*cells)[1].GetAdherence(), abs_error<double>::value);
  // EXPECT_NEAR(11, (*cells)[1].GetDiameter(), abs_error<double>::value);
  // EXPECT_NEAR(1.1, (*cells)[1].GetMass(), abs_error<double>::value);
}

}  // namespace neuroscience
}  // namespace bdm
