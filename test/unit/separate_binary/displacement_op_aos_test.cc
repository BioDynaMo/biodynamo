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

#include "compile_time_param.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/displacement_op_test.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam {
  template <typename TTBackend>
  using Self = CompileTimeParam<TTBackend>;
  using Backend = TBackend;
  using SimulationBackend = Scalar;
  using BiologyModules = Variant<NullBiologyModule>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

namespace displacement_op_test_internal {

TEST(DisplacementOpTest, ComputeAos) { RunTest(); }

}  // namespace displacement_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
