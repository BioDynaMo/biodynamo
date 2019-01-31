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

#ifndef UNIT_SEPARATE_BINARY_SIM_OBJECT_TEST_H_
#define UNIT_SEPARATE_BINARY_SIM_OBJECT_TEST_H_

#include <gtest/gtest.h>
#include "core/param/compile_time_param.h"
#include "core/sim_object/cell.h"
#include "core/sim_object/sim_object.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {

static bool gCellDivisionEventCtorCalled = false;
static bool gCellDivisionEventEventHandlerCalled = false;

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

  void EventHandler(const CellDivisionEvent& event,
                    CopyTestBiologyModule* other) {}

  EventId expected_event_;
  bool copy_ = true;
  BDM_CLASS_DEF_NV(CopyTestBiologyModule, 1);
};

/// Helper class to test remove visitor
struct RemoveTestBiologyModule {
  RemoveTestBiologyModule() {}

  RemoveTestBiologyModule(const CellDivisionEvent& event,
                          RemoveTestBiologyModule* other) {}

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
  BDM_CLASS_DEF_NV(RemoveTestBiologyModule, 1);
};

/// Helper class to test if the EventHandler is called
struct EventHandlerBm {
  EventHandlerBm() {}

  EventHandlerBm(const CellDivisionEvent& event, EventHandlerBm* other) {}

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
  BDM_CLASS_DEF_NV(EventHandlerBm, 1);
};

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimObjectTypes = CTList<Cell, TestSimObject>;
  BDM_DEFAULT_CTPARAM_FOR(TestSimObject) {
    using BiologyModules =
        CTList<CopyTestBiologyModule, RemoveTestBiologyModule, EventHandlerBm>;
  };
};

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SIM_OBJECT_TEST_H_
