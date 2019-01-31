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

#include "unit/separate_binary/so_pointer_aos_test.h"

namespace bdm {
namespace so_pointer_aos_test_internal {

TEST_F(IOTest, SoPointerRmContainer_Aos) {
  Simulation sim(TEST_NAME);
  IOTestSoPointerRmContainerAos(&sim);
}

TEST(SoPointerTest, Basics) {
  Simulation simulation(TEST_NAME);
  SoPointerTestClass so(123);
  SoPointerTest<SoPointerTestClass, Scalar>(so);
}

TEST_F(IOTest, SoPointerNullptr) {
  Simulation simulation(TEST_NAME);
  IOTestSoPointerNullptr();
}

}  // namespace so_pointer_aos_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
