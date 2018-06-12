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

#include "bdm_imp.h"
#include "cell.h"
#include "compile_time_param.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {
  using SimulationBackend = Scalar;
};

namespace simulation_object_util_test_aos_internal {

TEST(SimulationObjectUtilAosTest, RemoveFromSimulation) {
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  auto* cells = rm->Get<Cell>();

  cells->push_back(Cell());
  EXPECT_EQ(1u, cells->size());

  auto&& to_be_removed = (*cells)[0];
  to_be_removed.RemoveFromSimulation();
  cells->Commit();
  EXPECT_EQ(0u, cells->size());
}

TEST(SimulationObjectUtilAosTest, GetSoPtr) {
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  for (uint64_t i = 0; i < 10; i++) {
    rm->New<Cell>(1);
  }

  rm->Get<Cell>()->Commit();
  EXPECT_EQ(10u, rm->GetNumSimObjects());

  auto cells = rm->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetSoPtr().GetElementIdx());
  }
}

}  // namespace simulation_object_util_test_aos_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
