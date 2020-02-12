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

#include "core/sim_object/so_uid_generator.h"
#include <gtest/gtest.h>
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {

TEST(SoUidGeneratorTest, NormalAndDefragmentationMode) {
  Simulation simulation(TEST_NAME);
  simulation.GetResourceManager()->push_back(new TestSimObject(0));

  SoUidGenerator generator;

  EXPECT_EQ(SoUid(0), generator.NewSoUid());
  EXPECT_EQ(SoUid(1), generator.NewSoUid());
  EXPECT_EQ(SoUid(2), generator.NewSoUid());

  // increment simulated time steps
  simulation.GetScheduler()->Simulate(1);

  // defragmentation mode
  SoUidMap<SoHandle> map(3);
  map.Insert(SoUid(1), SoHandle(123));
  // slots 0, and 2 are empty

  generator.EnableDefragmentation(&map);
  EXPECT_EQ(SoUid(0, 1), generator.NewSoUid());
  EXPECT_EQ(SoUid(2, 1), generator.NewSoUid());
  // no more empty slots -> generator should have switched back to normal mode
  EXPECT_EQ(SoUid(3, 0), generator.NewSoUid());
}

#ifdef USE_DICT
TEST_F(IOTest, SoUidGenerator) {
  SoUidGenerator test;
  test.NewSoUid();
  test.NewSoUid();
  test.NewSoUid();

  SoUidGenerator* restored = nullptr;

  BackupAndRestore(test, &restored);

  EXPECT_EQ(restored->GetHighestIndex(), 3u);
  EXPECT_EQ(restored->NewSoUid(), SoUid(3u));

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
