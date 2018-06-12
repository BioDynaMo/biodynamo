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

#include "unit/simulation_object_test.h"
#include "cell.h"
#include "unit/default_ctparam.h"

namespace bdm {
namespace simulation_object_test_internal {

TEST(SimulationObjectTest, push_backAndClear) { RunPushBackAndClearTest(); }

TEST(SimulationObjectTest, SoaGetElementIndex) {
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  for (uint64_t i = 0; i < 10; i++) {
    rm->New<Cell>(1);
  }
  rm->Get<Cell>()->Commit();
  EXPECT_EQ(10u, rm->GetNumSimObjects());
  auto cells = rm->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetElementIdx());
  }
}

TEST(SimulationObjectTest, Clear) {
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  for (uint64_t i = 0; i < 10; i++) {
    rm->New<Cell>(1);
  }
  auto* cells = rm->Get<Cell>();
  cells->Commit();

  EXPECT_EQ(10u, cells->size());

  cells->DelayedRemove(5);

  EXPECT_EQ(10u, cells->size());
  cells->clear();

  // this would segfault if `TransactionalVector::to_be_removed_` will not be
  // cleared
  cells->Commit();
}

}  // namespace simulation_object_test_internal
}  // namespace bdm
