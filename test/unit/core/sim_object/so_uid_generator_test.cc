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

#include <gtest/gtest.h>
#include "core/sim_object/so_uid_generator.h"
#include "unit/test_util/io_test.h"

namespace bdm {

#ifdef USE_DICT
TEST_F(IOTest, SoUidGenerator) {
  SoUidGenerator test;
  test.NewSoUid();
  test.NewSoUid();
  test.NewSoUid();

  SoUidGenerator* restored = nullptr;

  BackupAndRestore(test, &restored);

  EXPECT_EQ(restored->GetHighestIndex(), 3u);

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
