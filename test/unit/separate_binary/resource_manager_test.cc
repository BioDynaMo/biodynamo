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

// I/O related code must be in header file
#include "unit/separate_binary/resource_manager_test.h"
#include "simulation_implementation.h"
#include "unit/io_test.h"

namespace bdm {

TEST(ResourceManagerTest, Get) {
  RunGetTest<ResourceManager<>, A, B>();
  RunGetTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnElement) {
  RunApplyOnElementTest<ResourceManager<>, A, B>();
  RunApplyOnElementTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnAllElements) {
  RunApplyOnAllElementsTest<ResourceManager<>, A, B>();
  RunApplyOnAllElementsTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, GetNumSimObjects) {
  RunGetNumSimObjects<ResourceManager<>, A, B>();
  RunGetNumSimObjects<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsParallel) {
  RunApplyOnAllElementsParallelTest<ResourceManager<>, B>();
}

TEST(ResourceManagerTest, ApplyOnAllTypes) {
  RunApplyOnAllTypesTest<ResourceManager<>, A, B>();
  RunApplyOnAllTypesTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, IO) { RunIOTest(); }

TEST(ResourceManagerTest, GetTypeIndex) {
  RunGetTypeIndexTest<ResourceManager<>, A, B>();
  RunGetTypeIndexTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, push_back) {
  RunPushBackTest<ResourceManager<>, A, B>();
  RunPushBackTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, RemoveAndContains) {
  RunRemoveAndContainsTest<ResourceManager<>, A, B>();
  RunRemoveAndContainsTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, Clear) {
  RunClearTest<ResourceManager<>, A, B>();
  RunClearTest<ResourceManager<>, SoaA, SoaB>();
}

TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallel) {
  RunSortAndApplyOnAllElementsParallel<A, B>();
}

TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallelDynamic) {
  RunSortAndApplyOnAllElementsParallelDynamic<A, B>();
}

TEST(ResourceManagerTest, DiffusionGrid) {
  ResourceManager<> rm;

  int counter = 0;
  auto count = [&](DiffusionGrid* dg) { counter++; };

  DiffusionGrid* dgrid_1 = new DiffusionGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid(1, "Natrium", 0.2, 0.1, 1);
  DiffusionGrid* dgrid_3 = new DiffusionGrid(2, "Calcium", 0.5, 0.1, 1);
  rm.AddDiffusionGrid(dgrid_1);
  rm.AddDiffusionGrid(dgrid_2);
  rm.AddDiffusionGrid(dgrid_3);

  rm.ApplyOnAllDiffusionGrids(count);
  ASSERT_EQ(3, counter);

  EXPECT_EQ(dgrid_1, rm.GetDiffusionGrid(0));
  EXPECT_EQ(dgrid_1, rm.GetDiffusionGrid("Kalium"));

  EXPECT_EQ(dgrid_2, rm.GetDiffusionGrid(1));
  EXPECT_EQ(dgrid_2, rm.GetDiffusionGrid("Natrium"));

  EXPECT_EQ(dgrid_3, rm.GetDiffusionGrid(2));
  EXPECT_EQ(dgrid_3, rm.GetDiffusionGrid("Calcium"));

  rm.RemoveDiffusionGrid(dgrid_2->GetSubstanceId());

  counter = 0;
  rm.ApplyOnAllDiffusionGrids(count);
  ASSERT_EQ(2, counter);
}

TEST(ResourceManagerTest, RunGetSimObjectTest) { RunGetSimObjectTest<A, B>(); }

TEST(SoHandleTest, Getters) {
  SoHandle so_handle(1,2,3);

  EXPECT_EQ(1u, so_handle.GetNumaNode());
  EXPECT_EQ(2u, so_handle.GetTypeIdx());
  EXPECT_EQ(3u, so_handle.GetElementIdx());
}

TEST(SoHandleTest, EqualsOperator) {
  EXPECT_EQ(SoHandle(0, 0, 0), SoHandle(0, 0, 0));
  EXPECT_EQ(SoHandle(0, 1, 0), SoHandle(0, 1, 0));
  EXPECT_EQ(SoHandle(0, 0, 1), SoHandle(0, 0, 1));
  EXPECT_EQ(SoHandle(0, 1, 1), SoHandle(0, 1, 1));
  EXPECT_EQ(SoHandle(1, 0, 0), SoHandle(1, 0, 0));
  EXPECT_EQ(SoHandle(1, 1, 0), SoHandle(1, 1, 0));
  EXPECT_EQ(SoHandle(1, 0, 1), SoHandle(1, 0, 1));
  EXPECT_EQ(SoHandle(1, 1, 1), SoHandle(1, 1, 1));

  EXPECT_FALSE(SoHandle(0, 0, 0) == SoHandle(0, 0, 1));
  EXPECT_FALSE(SoHandle(0, 0, 0) == SoHandle(0, 1, 0));
  EXPECT_FALSE(SoHandle(0, 0, 0) == SoHandle(1, 0, 0));
}

TEST_F(IOTest, SoHandle) {
  SoHandle h(12, 34);
  SoHandle* restored = nullptr;

  BackupAndRestore(h, &restored);

  EXPECT_EQ(12u, restored->GetTypeIdx());
  EXPECT_EQ(34u, restored->GetElementIdx());
}

}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
