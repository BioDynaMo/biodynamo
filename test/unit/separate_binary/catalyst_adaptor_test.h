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
#include "cell.h"
#include "compile_time_param.h"
#include "io_util.h"
#include "model_initializer.h"
#include "simulation_implementation.h"
#include "simulation_object_util.h"
#include "unit/test_util.h"
#include "visualization/catalyst_adaptor.h"

// TODO(lukas) move file to unit/visualization

namespace bdm {
/// Test fixture for catalyst adaptor test to eliminate side effects
class CatalystAdaptorTest : public ::testing::Test {
 protected:
  static constexpr char const* kSimulationName = "MySimulation";
  static constexpr char const* kSimulationInfoJson =
      "output/MySimulation/simulation_info.json";
  static constexpr char const* kParaviewState =
      "output/MySimulation/MySimulation.pvsm";
  Simulation<>* simulation_;

  virtual void SetUp() {
    Simulation<>::counter_ = 0;
    simulation_ = new Simulation<>(kSimulationName);
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }

  virtual void TearDown() {
    delete simulation_;
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }
};

BDM_SIM_OBJECT(MyCell, Cell) {
  BDM_SIM_OBJECT_HEADER(MyCellExt, 1, dummmy_);

 public:
  MyCellExt() {}
  vec<int> dummmy_;
};

BDM_SIM_OBJECT(MyNeuron, Cell) {
  BDM_SIM_OBJECT_HEADER(MyNeuronExt, 1, dummmy_);

 public:
  MyNeuronExt() {}
  vec<int> dummmy_;
};

template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using AtomicTypes = VariadicTypedef<Cell, MyCell, MyNeuron>;
};

}  // namespace bdm
