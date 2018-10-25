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

#ifndef UNIT_BIOLOGY_MODULE_UTIL_TEST_H_
#define UNIT_BIOLOGY_MODULE_UTIL_TEST_H_

#include "biology_module_util.h"
#include "cell.h"
#include "gtest/gtest.h"

namespace bdm {

struct TestSimulationObject {};

static bool gRunMethodCalled = false;

/// Helper class to test run visitor
template <typename TSimulationObject>
struct RunTestBiologyModule {
  template <typename T>
  void Run(T* t) {
    EXPECT_EQ(expected_run_parameter_, t);
    gRunMethodCalled = true;
  }

  bool Copy(EventId event) {
    EXPECT_TRUE(false) << "This method should not be called from RunVisitor";
    return false;
  }

  bool Remove(EventId event) const {
    EXPECT_TRUE(false) << "This method should not be called";
    return false;
  }

  TSimulationObject* expected_run_parameter_ = nullptr;
  BDM_CLASS_DEF_NV(RunTestBiologyModule, 1);
};

inline void RunRunVisitor() {
  TestSimulationObject sim_object;
  RunVisitor<TestSimulationObject> visitor(&sim_object);

  RunTestBiologyModule<TestSimulationObject> module;
  module.expected_run_parameter_ = &sim_object;
  Variant<RunTestBiologyModule<TestSimulationObject>> variant = module;

  visit(visitor, variant);
  EXPECT_TRUE(gRunMethodCalled);
}

}  // namespace bdm

#endif  // UNIT_BIOLOGY_MODULE_UTIL_TEST_H_
