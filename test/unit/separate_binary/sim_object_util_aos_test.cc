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

#include "gtest/gtest.h"

#include "core/param/compile_time_param.h"
#include "core/sim_object/cell.h"
#include "core/simulation_implementation.h"
#include "unit/test_util/test_util.h"

namespace bdm {

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimulationBackend = Scalar;
};

namespace simulation_object_util_test_aos_internal {

TEST(SimObjectUtilAosTest, GetSoPtr) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  for (uint64_t i = 0; i < 10; i++) {
    rm->push_back(Cell(1));
  }

  EXPECT_EQ(10u, rm->GetNumSimObjects());

  const auto* cells = rm->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    SoPointer<Cell, Scalar> expected((*cells)[i].GetUid());
    EXPECT_EQ(expected, (*cells)[i].GetSoPtr());
  }
}

}  // namespace simulation_object_util_test_aos_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
