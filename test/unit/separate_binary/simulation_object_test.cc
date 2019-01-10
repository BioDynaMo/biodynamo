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
    rm->push_back(Cell(1));
  }
  EXPECT_EQ(10u, rm->GetNumSimObjects());
  const auto* cells = rm->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetElementIdx());
  }
}

TEST(SimulationObjectTest, Clear) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  SoUid remove_uid;

  for (uint64_t i = 0; i < 10; i++) {
    Cell cell(1);
    rm->push_back(cell);
    if (i == 5) {
      remove_uid = cell.GetUid();
    }
  }

  EXPECT_EQ(10u, rm->GetNumSimObjects());

  rm->Remove(remove_uid);

  EXPECT_EQ(9u, rm->GetNumSimObjects());

  rm->Clear();
  EXPECT_EQ(0u, rm->GetNumSimObjects());
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
