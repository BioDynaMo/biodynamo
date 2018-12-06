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

#include "unit/separate_binary/simulation_object_test.h"
#include "simulation_implementation.h"
#include "unit/test_util.h"

namespace bdm {

TEST(SimulationObjectTest, SoaGetElementIndex) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

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
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  for (uint64_t i = 0; i < 10; i++) {
    rm->New<Cell>(1);
  }
  auto* cells = rm->Get<Cell>();
  cells->Commit();

  EXPECT_EQ(10u, cells->size());

  // FIXME
  // cells->DelayedRemove(5);

  EXPECT_EQ(10u, cells->size());
  cells->clear();

  // this would segfault if `TransactionalVector::to_be_removed_` will not be
  // cleared
  cells->Commit();
}

TEST(SimulationObjectTest, CopyBiologyModules) {
  // set-up
  TestSimObject src;
  CellDivisionEvent event;
  CopyTestBiologyModule module;
  module.expected_event_ = event.kEventId;
  src.AddBiologyModule(module);
  gCellDivisionEventCtorCalled = false;

  // Copy
  TestSimObject dest(event, &src);

  // verify
  EXPECT_EQ(1u, dest.GetAllBiologyModules().size());
  EXPECT_TRUE(gCellDivisionEventCtorCalled);
  bool foo =
      get_if<CopyTestBiologyModule>(&dest.GetAllBiologyModules()[0])->copy_;
  EXPECT_TRUE(foo);
}

TEST(SimulationObjectTest, CopyBiologyModulesIsNotCopied) {
  // set-up
  TestSimObject src;
  CellDivisionEvent event;
  CopyTestBiologyModule module;
  module.copy_ = false;
  module.expected_event_ = event.kEventId;
  src.AddBiologyModule(module);
  gCellDivisionEventCtorCalled = false;

  // Copy
  TestSimObject dest(event, &src);

  // verify
  EXPECT_EQ(0u, dest.GetAllBiologyModules().size());
  EXPECT_FALSE(gCellDivisionEventCtorCalled);
}

TEST(SimulationObjectTest, Remove) {
  // set-up
  TestSimObject src;
  TestSimObject dest;
  CellDivisionEvent event;
  RemoveTestBiologyModule module;
  module.expected_event_ = event.kEventId;
  src.AddBiologyModule(module);
  dest.AddBiologyModule(module);

  // call
  src.EventHandler(event, &dest);

  // verify
  EXPECT_EQ(0u, src.GetAllBiologyModules().size());
  EXPECT_EQ(1u, dest.GetAllBiologyModules().size());
}

TEST(SimulationObjectTest, EventHandler) {
  // set-up
  TestSimObject src;
  TestSimObject dest;
  CellDivisionEvent event;
  EventHandlerBm module;
  module.expected_event_ = event.kEventId;
  src.AddBiologyModule(module);
  dest.AddBiologyModule(module);
  gCellDivisionEventEventHandlerCalled = false;

  // call
  src.EventHandler(event, &dest);

  // verify
  EXPECT_EQ(1u, src.GetAllBiologyModules().size());
  EXPECT_EQ(1u, dest.GetAllBiologyModules().size());
  EXPECT_TRUE(gCellDivisionEventEventHandlerCalled);
}

}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
