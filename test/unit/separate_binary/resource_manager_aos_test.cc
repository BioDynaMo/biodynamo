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
#include "unit/separate_binary/resource_manager_aos_test.h"
#include "core/sim_object/cell.h"
#include "core/simulation_implementation.h"
#include "unit/test_util/io_test.h"

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

TEST(ResourceManagerTest, RunGetSimObjectTest) { RunGetSimObjectTest<A, B>(); }

TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallel) {
  RunSortAndApplyOnAllElementsParallel<A, B>();
}

TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallelDynamic) {
  RunSortAndApplyOnAllElementsParallelDynamic<A, B>();
}

}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
