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

#include "unit/core/sim_object/so_pointer_test.h"
#include "unit/test_util/io_test.h"

namespace bdm {
namespace so_pointer_test_internal {

TEST(SoPointerTest, Basics) {
  Simulation simulation(TEST_NAME);

  SoPointer<TestSimObject> null_so_pointer;
  EXPECT_TRUE(null_so_pointer == nullptr);


  TestSimObject* so = new TestSimObject();
  so->SetData(123);
  simulation.GetResourceManager()->push_back(so);

  SoPointer<TestSimObject> so_ptr(so->GetUid());

  EXPECT_TRUE(so_ptr != nullptr);
  EXPECT_EQ(123, so_ptr->GetData());

  so_ptr = nullptr;
  EXPECT_TRUE(so_ptr == nullptr);
}

#ifdef USE_DICT

TEST_F(IOTest, SoPointer) {
  Simulation simulation(TEST_NAME);
  RunIOTest(&simulation);
}

TEST_F(IOTest, SoPointerNullptr) {
  Simulation simulation(TEST_NAME);
  IOTestSoPointerNullptr();
}

#endif  // USE_DICT

}  // namespace so_pointer_test_internal
}  // namespace bdm
