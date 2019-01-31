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

#ifndef UNIT_CORE_OPERATION_DIVIDING_CELL_OP_TEST_H_
#define UNIT_CORE_OPERATION_DIVIDING_CELL_OP_TEST_H_

#include <omp.h>
#include "core/operation/dividing_cell_op.h"
#include "core/sim_object/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

template <typename TCell, typename TSimulation = Simulation<>>
void RunTest() {
  TSimulation simulation("dividing_cell_op_test_RunTest");
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();
  ctxt->SetupIteration();

  auto* cells = rm->template Get<TCell>();

  TCell cell_0(41.0);
  TCell cell_1(19.0);
  double volume_mother = cell_0.GetVolume();

  rm->push_back(cell_0);
  rm->push_back(cell_1);

  EXPECT_EQ(2u, rm->GetNumSimObjects());

  DividingCellOp op;
  op();

  ctxt->TearDownIteration();

  ASSERT_EQ(3u, rm->GetNumSimObjects());
  EXPECT_NEAR(19.005288996600001, (*cells)[1].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(3594.3640018287319, (*cells)[1].GetVolume(),
              abs_error<double>::value);

  // cell got divided so it must be smaller than before
  // more detailed division test can be found in `cell_test.h`
  EXPECT_GT(41, (*cells)[0].GetDiameter());
  EXPECT_GT(41, (*cells)[2].GetDiameter());

  // volume of two daughter cells must be equal to volume of the mother
  EXPECT_NEAR(volume_mother, (*cells)[0].GetVolume() + (*cells)[2].GetVolume(),
              abs_error<double>::value);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_DIVIDING_CELL_OP_TEST_H_
