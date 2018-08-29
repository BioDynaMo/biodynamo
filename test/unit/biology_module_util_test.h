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
namespace biology_module_util_test_internal {

static bool gRunMethodCalled = false;
static bool gCellDivisionEventCtorCalled = false;
static bool gCellDivisionEventEventHandlerCalled = false;

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
  ClassDefNV(RunTestBiologyModule, 1);
};

/// Helper class to test copy visitor
struct CopyTestBiologyModule {
  CopyTestBiologyModule() {}

  CopyTestBiologyModule(const CellDivisionEvent& event,
                        CopyTestBiologyModule* other) {
    gCellDivisionEventCtorCalled = true;
    expected_event_ = event.kEventId;
    copy_ = other->copy_;
  }

  template <typename T>
  void Run(T* t) {
    EXPECT_TRUE(false) << "This method should not be called";
  }

  bool Copy(EventId event) const {
    EXPECT_EQ(expected_event_, event);
    return copy_;
  }

  bool Remove(EventId event) const {
    EXPECT_TRUE(false) << "This method should not be called";
    return false;
  }

  EventId expected_event_;
  bool copy_ = true;
  ClassDefNV(CopyTestBiologyModule, 1);
};

/// Helper class to test remove visitor
struct RemoveTestBiologyModule {
  RemoveTestBiologyModule() {}

  template <typename T>
  void Run(T* t) {
    EXPECT_TRUE(false) << "This method should not be called";
  }

  bool Copy(EventId event) const {
    EXPECT_TRUE(false) << "This method should not be called";
    return false;
  }

  bool Remove(EventId event) const {
    EXPECT_EQ(expected_event_, event);
    return expected_event_ == event;
  }

  void EventHandler(const CellDivisionEvent& event,
                    RemoveTestBiologyModule* other) {
    EXPECT_TRUE(false)
        << "This method should not be called, since this bm will be removed";
  }

  EventId expected_event_;
  ClassDefNV(RemoveTestBiologyModule, 1);
};

/// Helper class to test if the EventHandler is called
struct EventHandlerBm {
  EventHandlerBm() {}

  template <typename T>
  void Run(T* t) {
    EXPECT_TRUE(false) << "This method should not be called";
  }

  bool Copy(EventId event) const { return true; }

  bool Remove(EventId event) const {
    EXPECT_EQ(expected_event_, event);
    return false;
  }

  void EventHandler(const CellDivisionEvent& event, EventHandlerBm* other) {
    gCellDivisionEventEventHandlerCalled = true;
  }

  EventId expected_event_;
  ClassDefNV(EventHandlerBm, 1);
};

struct TestSimulationObject {};

inline void RunRunVisitor() {
  TestSimulationObject sim_object;
  RunVisitor<TestSimulationObject> visitor(&sim_object);

  RunTestBiologyModule<TestSimulationObject> module;
  module.expected_run_parameter_ = &sim_object;
  Variant<RunTestBiologyModule<TestSimulationObject>> variant = module;

  visit(visitor, variant);
  EXPECT_TRUE(gRunMethodCalled);
}

}  // namespace biology_module_util_test_internal
}  // namespace bdm

#endif  // UNIT_BIOLOGY_MODULE_UTIL_TEST_H_
