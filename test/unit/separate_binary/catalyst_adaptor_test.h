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

#ifndef UNIT_SEPARATE_BINARY_CATALYST_ADAPTOR_TEST_H_
#define UNIT_SEPARATE_BINARY_CATALYST_ADAPTOR_TEST_H_

#include <gtest/gtest.h>
#include "core/model_initializer.h"
#include "core/param/compile_time_param.h"
#include "core/sim_object/cell.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation_implementation.h"
#include "core/util/io.h"
#include "core/visualization/catalyst_adaptor.h"
#include "unit/test_util/test_util.h"

// TODO(lukas) move file to unit/visualization

namespace bdm {

BDM_SIM_OBJECT(MyCell, Cell) {
  BDM_SIM_OBJECT_HEADER(MyCell, Cell, 1, dummmy_);

 public:
  MyCellExt() {}
  vec<int> dummmy_;
};

BDM_SIM_OBJECT(MyNeuron, Cell) {
  BDM_SIM_OBJECT_HEADER(MyNeuron, Cell, 1, dummmy_);

 public:
  MyNeuronExt() {}
  vec<int> dummmy_;
};

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  BDM_DEFAULT_CTPARAM_FOR(MyCell){};
  BDM_DEFAULT_CTPARAM_FOR(MyNeuron){};
  using SimObjectTypes = CTList<Cell, MyCell, MyNeuron>;
};

/// Test fixture for catalyst adaptor test to eliminate side effects
class CatalystAdaptorTest : public ::testing::Test {
 protected:
  static constexpr char const* kSimulationName = "MySimulation";
  static constexpr char const* kSimulationInfoJson =
      "output/MySimulation/simulation_info.json";
  static constexpr char const* kParaviewState =
      "output/MySimulation/MySimulation.pvsm";

  virtual void SetUp() {
    Simulation::counter_ = 0;
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }

  virtual void TearDown() {
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }
};

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_CATALYST_ADAPTOR_TEST_H_
