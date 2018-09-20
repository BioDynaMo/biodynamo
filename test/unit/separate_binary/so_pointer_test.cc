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

#include "unit/separate_binary/so_pointer_test.h"
#include "simulation_implementation.h"
#include "unit/io_test.h"

namespace bdm {
namespace so_pointer_test_internal {

TEST(SoPointerTest, Aos) {
  TransactionalVector<SoPointerTestClass> sim_objects;
  SoPointerTestClass so(123);
  sim_objects.push_back(so);
  SoPointerTest<decltype(sim_objects), Scalar>(&sim_objects);
}

TEST(SoPointerTest, Soa) {
  auto sim_objects = SoPointerTestClass::NewEmptySoa();
  SoPointerTestClass so(123);
  sim_objects.push_back(so);
  SoPointerTest<decltype(sim_objects), Soa>(&sim_objects);
}

TEST_F(IOTest, SoPointerAnyContainer_Aos) {
  Simulation<> simulation(TEST_NAME);
  IOTestSoPointerAnyContainerAos();
}

TEST_F(IOTest, SoPointerAnyContainer_Soa) {
  Simulation<> simulation(TEST_NAME);
  IOTestSoPointerAnyContainerSoa();
}

TEST_F(IOTest, SoPointerRmContainer_Soa) {
  Simulation<> simulation(TEST_NAME);
  IOTestSoPointerRmContainerSoa(&simulation);
}

TEST_F(IOTest, SoPointerNullptr_Aos) {
  Simulation<> simulation(TEST_NAME);
  IOTestSoPointerNullptrAos();
}

TEST_F(IOTest, SoPointerNullptr_Soa) {
  Simulation<> simulation(TEST_NAME);
  IOTestSoPointerNullptrSoa();
}

}  // namespace so_pointer_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
