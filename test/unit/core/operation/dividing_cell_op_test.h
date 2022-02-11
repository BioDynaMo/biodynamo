// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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

#include "core/agent/cell.h"
#include "core/operation/dividing_cell_op.h"
#include "core/resource_manager.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

inline void RunTest() {
  Simulation simulation("dividing_cell_op_test_RunTest");
  auto* rm = simulation.GetResourceManager();
  auto* ctxt = simulation.GetExecutionContext();
  ctxt->SetupIterationAll(simulation.GetAllExecCtxts());

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  Cell* cell_0 = new Cell(41.0);
  Cell* cell_1 = new Cell(19.0);
  double volume_mother = cell_0->GetVolume();

  rm->AddAgent(cell_0);
  rm->AddAgent(cell_1);

  EXPECT_EQ(2u, rm->GetNumAgents());

  auto* op = NewOperation("DividingCellOp");
  rm->ForEachAgentParallel(*op);

  ctxt->TearDownIterationAll(simulation.GetAllExecCtxts());

  ASSERT_EQ(3u, rm->GetNumAgents());
  Cell* final_cell0 = dynamic_cast<Cell*>(rm->GetAgent(ref_uid + 0));
  Cell* final_cell1 = dynamic_cast<Cell*>(rm->GetAgent(ref_uid + 1));
  Cell* final_cell2 = dynamic_cast<Cell*>(rm->GetAgent(ref_uid + 2));
  EXPECT_NEAR(19.005288996600001, final_cell1->GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(3594.3640018287319, final_cell1->GetVolume(),
              abs_error<double>::value);

  // cell got divided so it must be smaller than before
  // more detailed division test can be found in `cell_test.h`
  EXPECT_GT(41, final_cell0->GetDiameter());
  EXPECT_GT(41, final_cell2->GetDiameter());

  // volume of two daughter cells must be equal to volume of the mother
  EXPECT_NEAR(volume_mother,
              final_cell0->GetVolume() + final_cell2->GetVolume(),
              abs_error<double>::value);

  delete op;
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_DIVIDING_CELL_OP_TEST_H_
