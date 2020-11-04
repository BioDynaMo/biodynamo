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

#ifndef UNIT_CORE_OPERATION_DISPLACEMENT_OP_TEST_H_
#define UNIT_CORE_OPERATION_DISPLACEMENT_OP_TEST_H_

#include "core/operation/displacement_op.h"
#include "core/agent/cell.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace displacement_op_test_internal {

inline void RunTest() {
  Simulation simulation("displacement_op_test_RunTest");
  auto* rm = simulation.GetResourceManager();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  // cell 0
  Cell* cell0 = new Cell();
  cell0->SetAdherence(0.3);
  cell0->SetDiameter(9);
  cell0->SetMass(1.4);
  cell0->SetPosition({0, 0, 0});
  rm->push_back(cell0);

  // cell 1
  Cell* cell1 = new Cell();
  cell1->SetAdherence(0.4);
  cell1->SetDiameter(11);
  cell1->SetMass(1.1);
  cell1->SetPosition({0, 5, 0});
  rm->push_back(cell1);

  simulation.GetEnvironment()->Update();

  // execute operation
  auto* ctxt = simulation.GetExecutionContext();
  auto* op = NewOperation("displacement");
  ctxt->Execute(rm->GetAgent(ref_uid), {op});
  ctxt->Execute(rm->GetAgent(ref_uid + 1), {op});

  // check results
  // cell 0
  Cell* final_cell0 = dynamic_cast<Cell*>(rm->GetAgent(ref_uid + 0));
  Cell* final_cell1 = dynamic_cast<Cell*>(rm->GetAgent(ref_uid + 1));
  auto final_position = final_cell0->GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 1
  final_position = final_cell1->GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0980452768658333, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 0
  auto final_tf = final_cell0->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 1
  final_tf = final_cell1->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 0
  EXPECT_NEAR(0.3, final_cell0->GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, final_cell0->GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, final_cell0->GetMass(), abs_error<double>::value);
  // cell 1
  EXPECT_NEAR(0.4, final_cell1->GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, final_cell1->GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, final_cell1->GetMass(), abs_error<double>::value);

  delete op;
}

}  // namespace displacement_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_DISPLACEMENT_OP_TEST_H_
