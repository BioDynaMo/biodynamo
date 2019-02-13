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

#ifndef UNIT_CORE_SIM_OBJECT_SO_POINTER_TEST_H_
#define UNIT_CORE_SIM_OBJECT_SO_POINTER_TEST_H_

#include <gtest/gtest.h>

#include "core/resource_manager.h"
#include "core/sim_object/so_pointer.h"
#include "core/simulation.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {
namespace so_pointer_test_internal {

inline void RunIOTest(Simulation* sim) {
  auto* rm = sim->GetResourceManager();
  rm->push_back(new TestSimObject(123));
  TestSimObject* so2 = new TestSimObject(456);
  rm->push_back(so2);

  SoPointer<TestSimObject> soptr(so2->GetUid());
  SoPointer<TestSimObject>* restored;

  BackupAndRestore(soptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(456, (*restored)->GetData());
}

inline void IOTestSoPointerNullptr() {
  SoPointer<TestSimObject> null_so_pointer;
  SoPointer<TestSimObject>* restored = nullptr;

  BackupAndRestore(null_so_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

}  // namespace so_pointer_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_SIM_OBJECT_SO_POINTER_TEST_H_
