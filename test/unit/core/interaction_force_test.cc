// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/interaction_force.h"
#include "core/agent/cell.h"
#include "gtest/gtest.h"
#include "neuroscience/module.h"
#include "neuroscience/neurite_element.h"
#include "unit/test_util/test_util.h"

namespace bdm {

using neuroscience::NeuriteElement;

/// Tests the forces that are created between the reference sphere and its
/// overlapping neighbors
/// implementation uses virual bigger radii to have distant interaction
TEST(InteractionForce, GeneralSphere) {
  Simulation simulation(TEST_NAME);

  Cell cell({1.1, 1.0, 0.9});
  cell.SetDiameter(8);
  Cell nb({0, 0, 0});
  nb.SetDiameter(5);

  InteractionForce force;
  auto result = force.Calculate(&cell, &nb);

  EXPECT_NEAR(7.1429184067241138, result[0], abs_error<double>::value);
  EXPECT_NEAR(6.4935621879310119, result[1], abs_error<double>::value);
  EXPECT_NEAR(5.8442059691379109, result[2], abs_error<double>::value);

  nb.SetDiameter(10);
  nb.SetPosition({5, 5, 0});
  result = force.Calculate(&cell, &nb);

  EXPECT_NEAR(-5.7454658831720176, result[0], abs_error<double>::value);
  EXPECT_NEAR(-5.892785521202069, result[1], abs_error<double>::value);
  EXPECT_NEAR(1.3258767422704656, result[2], abs_error<double>::value);
}

/// Tests the special case that non of the neighbors overlap
/// with the reference cell
TEST(InteractionForce, AllNonOverlappingSphere) {
  Simulation simulation(TEST_NAME);

  Cell cell({0, 0, 0});
  cell.SetDiameter(8);
  Cell nb({11.01, 0, 0});
  nb.SetDiameter(8);

  InteractionForce force;
  auto result = force.Calculate(&cell, &nb);

  EXPECT_NEAR(0, result[0], abs_error<double>::value);
  EXPECT_NEAR(0, result[1], abs_error<double>::value);
  EXPECT_NEAR(0, result[2], abs_error<double>::value);
}

/// Tests the special case that neighbor and reference cell
/// are at the same position -> should return random force
TEST(InteractionForce, AllAtSamePositionSphere) {
  // agent required for random number generator
  Simulation simulation(TEST_NAME);

  Cell cell({0, 0, 0});
  cell.SetDiameter(8);
  Cell nb({0, 0, 0});
  nb.SetDiameter(8);

  InteractionForce force;
  auto result = force.Calculate(&cell, &nb);

  // random number must be in interval [-3.0, 3.0]
  EXPECT_NEAR(0, result[0], 3);
  EXPECT_NEAR(0, result[1], 3);
  EXPECT_NEAR(0, result[2], 3);
}

/// Tests the forces that are created between the reference sphere and its
/// overlapping cylinder
TEST(DISABLED_Force, GeneralSphereCylinder) {
  // neuroscience::InitModule();
  // Simulation simulation(TEST_NAME);

  // auto* param = Simulation::GetActive()->GetParam();
  // std::cout << param->neurite_default_tension_ << std::endl;
  // FIXME
  // NeuriteElement cylinder;
  // cylinder.SetMassLocation({2, 0, 0});
  // cylinder.SetSpringAxis({-5, -1, -3});  // -> proximal end = {7, 1, 3}
  // cylinder.SetDiameter(4);
  // Cell sphere({0, 0, 0});
  // sphere.SetDiameter(10);
  //
  // EXPECT_ARR_NEAR({7, 1, 3}, cylinder.ProximalEnd());
  //
  // InteractionForce force;
  // auto result1 = force.Calculate(&cylinder, &sphere);
  //
  // EXPECT_NEAR(5, result1[0], abs_error<double>::value);
  // EXPECT_NEAR(0, result1[1], abs_error<double>::value);
  // EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  // EXPECT_NEAR(0, result1[3], abs_error<double>::value);
  //
  // auto result2 = force.Calculate(&sphere, &cylinder);
  //
  // EXPECT_ARR_NEAR({-5, 0, 0, 0}, result2);
}

TEST(DISABLED_Force, GeneralCylinder) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  NeuriteElement cylinder1;
  cylinder1.SetMassLocation({0, 0, 0});
  cylinder1.SetSpringAxis({-5, 0, 0});  // -> proximal end = {5, 0, 0}
  cylinder1.SetDiameter(4);

  EXPECT_ARR_NEAR({5, 0, 0}, cylinder1.ProximalEnd());

  NeuriteElement cylinder2;
  cylinder2.SetMassLocation({1, -2, 1});
  cylinder2.SetSpringAxis({-10, -4, -0.5});  // -> proximal end = {11, 2, 1.5}
  cylinder2.SetDiameter(6);

  EXPECT_ARR_NEAR({11, 2, 1.5}, cylinder2.ProximalEnd());

  InteractionForce force;
  auto result = force.Calculate(&cylinder1, &cylinder2);

  EXPECT_NEAR(-38.290598290598311, result[0], abs_error<double>::value);
  EXPECT_NEAR(9.5726495726495653, result[1], abs_error<double>::value);
  EXPECT_NEAR(-76.581196581196579, result[2], abs_error<double>::value);
  EXPECT_NEAR(1, result[3], abs_error<double>::value);

  result = force.Calculate(&cylinder2, &cylinder1);

  EXPECT_NEAR(38.290598290598311, result[0], abs_error<double>::value);
  EXPECT_NEAR(-9.5726495726495653, result[1], abs_error<double>::value);
  EXPECT_NEAR(76.581196581196579, result[2], abs_error<double>::value);
  EXPECT_NEAR(0.46153846153846156, result[3],
              abs_error<double>::value);  // FIXME not symmetric
}

TEST(InteractionForce, CylinderIntersectingAxis) {
  neuroscience::InitModule();
  // agent required for random number generator
  Simulation simulation(TEST_NAME);

  NeuriteElement cylinder1;
  cylinder1.SetMassLocation({0, 0, 0});
  cylinder1.SetSpringAxis({-5, 0, 0});  // -> proximal end = {5, 0, 0}
  cylinder1.SetDiameter(4);

  EXPECT_ARR_NEAR({5, 0, 0}, cylinder1.ProximalEnd());

  NeuriteElement cylinder2;
  cylinder2.SetMassLocation({2, -2, 0});
  cylinder2.SetSpringAxis({0, -4, 0});  // -> proximal end = {2, 2, 0}
  cylinder2.SetDiameter(6);

  EXPECT_ARR_NEAR({2, 2, 0}, cylinder2.ProximalEnd());

  InteractionForce force;
  auto result = force.Calculate(&cylinder1, &cylinder2);

  EXPECT_NEAR(0, result[0], 30);
  EXPECT_NEAR(0, result[1], 30);
  EXPECT_NEAR(0, result[2], 30);
  EXPECT_NEAR(0.4, result[3], abs_error<double>::value);

  result = force.Calculate(&cylinder2, &cylinder1);

  EXPECT_NEAR(0, result[0], 30);
  EXPECT_NEAR(0, result[1], 30);
  EXPECT_NEAR(0, result[2], 30);
  EXPECT_NEAR(0.5, result[3], abs_error<double>::value);
}

TEST(InteractionForce, NotTouchingParallelCylinders) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  NeuriteElement cylinder1;
  cylinder1.SetMassLocation({0, 0, 0});
  cylinder1.SetSpringAxis({-5, 0, 0});  // -> proximal end = {5, 0, 0}
  cylinder1.SetDiameter(4);

  EXPECT_ARR_NEAR({5, 0, 0}, cylinder1.ProximalEnd());

  NeuriteElement cylinder2;
  cylinder2.SetMassLocation({0, -5, 0});
  cylinder2.SetSpringAxis({-5, 0, 0});  // -> proximal end = {5, -5, 0}
  cylinder2.SetDiameter(6);

  EXPECT_ARR_NEAR({5, -5, 0}, cylinder2.ProximalEnd());

  InteractionForce force;
  auto result = force.Calculate(&cylinder1, &cylinder2);

  EXPECT_NEAR(0, result[0], abs_error<double>::value);
  EXPECT_NEAR(0, result[1], abs_error<double>::value);
  EXPECT_NEAR(0, result[2], abs_error<double>::value);
  EXPECT_NEAR(0.5, result[3], abs_error<double>::value);

  result = force.Calculate(&cylinder2, &cylinder1);

  EXPECT_NEAR(0, result[0], abs_error<double>::value);
  EXPECT_NEAR(0, result[1], abs_error<double>::value);
  EXPECT_NEAR(0, result[2], abs_error<double>::value);
  EXPECT_NEAR(0.5, result[3], abs_error<double>::value);
}

// test I case of ForceOnACylinderFromASphere() function, ie if cylinder length
// < sphere radius
// sphere-cylinder interaction is done at the center and in the horizontal
// orientation of the cylinder
TEST(InteractionForce, SphereSmallCylinderHorizontal) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(50);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({-4, 24.5, 0});
  cylinder.SetSpringAxis({-8, 0, 0});  // -> proximal end = {4, 24.5, 0}
  cylinder.SetDiameter(4);

  EXPECT_ARR_NEAR({4, 24.5, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(-0.196774255282483, result1[0], abs_error<double>::value);
  EXPECT_NEAR(2.41048462721042, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(0, result1[3], abs_error<double>::value);
  auto result2 = force.Calculate(&sphere, &cylinder);
  EXPECT_ARR_NEAR4({0.196774255282483, -2.41048462721042, 0, 0}, result2);
}

// test I case of ForceOnACylinderFromASphere() function, ie if cylinder length
// < sphere radius
// sphere-cylinder interaction is done vertically at the tip of the cylinder
// (mass location)
TEST(InteractionForce, SphereSmallCylinderVertical) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(50);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({0, 24, 0});
  cylinder.SetSpringAxis({0, -8, 0});  // -> proximal end = {0, 32, 0}
  cylinder.SetDiameter(4);

  EXPECT_ARR_NEAR({0, 32, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(0, result1[0], abs_error<double>::value);
  EXPECT_NEAR(1, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(0, result1[3], abs_error<double>::value);

  auto result2 = force.Calculate(&sphere, &cylinder);
  EXPECT_ARR_NEAR4({0, -1, 0, 0}, result2);
}

// opposit case of Vertical: cylinder is below the cell
TEST(InteractionForce, SphereSmallCylinderVertical2) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  // auto* rm = simulation.GetResourceManager();

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(50);
  // rm->AddAgent(sphere);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({0, -24, 0});
  cylinder.SetSpringAxis({0, 8, 0});  // -> proximal end = {0, -32, 0}
  cylinder.SetDiameter(4);

  EXPECT_ARR_NEAR({0, -32, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(0, result1[0], abs_error<double>::value);
  EXPECT_NEAR(-1, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(0, result1[3], abs_error<double>::value);

  auto result2 = force.Calculate(&sphere, &cylinder);
  EXPECT_ARR_NEAR4({0, 1, 0, 0}, result2);
}

// test the II case of ForceOnACylinderFromASphere() function, ie if cylinder
// length > sphere radius
// sphere-cylinder interaction is done at the center and in the horizontal
// orientation of the cylinder
TEST(InteractionForce, SphereLongCylinderHorizontalCenter) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(10);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({-10, 14.5, 0});
  cylinder.SetSpringAxis({-20, 0, 0});  // -> proximal end = {10, 14.5, 0}
  cylinder.SetDiameter(20);

  EXPECT_ARR_NEAR({10, 14.5, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(0, result1[0], abs_error<double>::value);
  EXPECT_NEAR(0.5, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(
      0.5, result1[3],
      abs_error<double>::value);  // 0.5 force is transmited to proximalEnd

  auto result2 = force.Calculate(&sphere, &cylinder);
  EXPECT_ARR_NEAR4({0, -0.5, 0, 0}, result2);
}

// test the II case of ForceOnACylinderFromASphere() function, ie if cylinder
// length > sphere radius
// sphere-cylinder interaction is done at the proximal end and in the horizontal
// orientation of the cylinder
TEST(InteractionForce, SphereLongCylinderHorizontalpP) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(10);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({-19.5, 14.5, 0});
  cylinder.SetSpringAxis({-20, 0, 0});  // -> proximal end = {0.5, 14.5, 0}
  cylinder.SetDiameter(20);

  EXPECT_ARR_NEAR({0.5, 14.5, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(0, result1[0], abs_error<double>::value);
  EXPECT_NEAR(0.5, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(
      0.975, result1[3],
      abs_error<double>::value);  // 0.975 force is transmited to proximalEnd

  auto result2 = force.Calculate(&sphere, &cylinder);
  EXPECT_ARR_NEAR4({0, -0.5, 0, 0}, result2);
}

// test the II case of ForceOnACylinderFromASphere() function, ie if cylinder
// length > sphere radius
// sphere-cylinder interaction is done at the distal point and in the horizontal
// orientation of the cylinder
TEST(InteractionForce, SphereLongCylinderHorizontalpD) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);

  Cell sphere({0, 0, 0});
  sphere.SetDiameter(10);

  NeuriteElement cylinder;
  cylinder.SetMassLocation({-0.5, 14.5, 0});
  cylinder.SetSpringAxis({-20, 0, 0});  // -> proximal end = {19.5, 14.5, 0}
  cylinder.SetDiameter(20);

  EXPECT_ARR_NEAR({19.5, 14.5, 0}, cylinder.ProximalEnd());

  InteractionForce force;
  auto result1 = force.Calculate(&cylinder, &sphere);

  EXPECT_NEAR(0, result1[0], abs_error<double>::value);
  EXPECT_NEAR(0.5, result1[1], abs_error<double>::value);
  EXPECT_NEAR(0, result1[2], abs_error<double>::value);
  EXPECT_NEAR(
      0.025, result1[3],
      abs_error<double>::value);  // 0.025 force is transmited to proximalEnd

  auto result2 = force.Calculate(&sphere, &cylinder);

  EXPECT_ARR_NEAR4({0, -0.5, 0, 0}, result2);
}

}  // namespace bdm
