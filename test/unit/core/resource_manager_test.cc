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
#include "unit/core/resource_manager_test.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {

TEST(ResourceManagerTest, ApplyOnAllElements) { RunApplyOnAllElementsTest(); }

TEST(ResourceManagerTest, GetNumSimObjects) { RunGetNumSimObjects(); }

TEST(ResourceManagerTest, ApplyOnAllElementsParallel) {
  RunApplyOnAllElementsParallelTest();
}

#ifdef USE_DICT
TEST(ResourceManagerTest, IO) { RunIOTest(); }
#endif  // USE_DICT

TEST(ResourceManagerTest, PushBackAndGetSimObjectTest) {
  RunPushBackAndGetSimObjectTest();
}

TEST(ResourceManagerTest, RemoveAndContains) { RunRemoveAndContainsTest(); }

TEST(ResourceManagerTest, Clear) { RunClearTest(); }

// FIXME uncomment
// TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallel) {
//   RunSortAndApplyOnAllElementsParallel();
// }

TEST(ResourceManagerTest, SortAndApplyOnAllElementsParallelDynamic) {
  RunSortAndApplyOnAllElementsParallelDynamic();
}

TEST(ResourceManagerTest, DiffusionGrid) {
  ResourceManager rm;

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

TEST(ResourceManagerTest, TurnOnOffSoUidDefragmentation) {
  auto set_param = [](Param* param) {
    param->souid_defragmentation_low_watermark_ = 0.3;
    param->souid_defragmentation_high_watermark_ = 0.8;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* so_uid_generator = simulation.GetSoUidGenerator();

  // utilization = 0.5 > low watermark -> don't defragment
  //   create 10 objects
  for (uint64_t i = 0; i < 10; i++) {
    rm->push_back(new TestSimObject());
  }
  //   remove 5
  for (uint64_t i = 0; i < 5; i++) {
    rm->Remove(SoUid(i));
  }
  rm->EndOfIteration();
  EXPECT_FALSE(so_uid_generator->IsInDefragmentationMode());

  // utilization 0.2 < low watermark -> turn on defragmentation
  rm->Remove(SoUid(5));
  rm->Remove(SoUid(6));
  rm->Remove(SoUid(7));
  rm->EndOfIteration();
  EXPECT_FALSE(so_uid_generator->IsInDefragmentationMode());

  // utilization < low watermark -> turn off defragmentation
  for (uint64_t i = 0; i < 6; i++) {
    rm->push_back(new TestSimObject());
  }
  rm->EndOfIteration();
  EXPECT_FALSE(so_uid_generator->IsInDefragmentationMode());
}

}  // namespace bdm
