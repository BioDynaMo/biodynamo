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

#include "core/grid.h"
#include "core/operation/displacement_op.h"
#include "core/sim_object/cell.h"
#include "core/simulation_implementation.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace displacement_op_test_internal {

template <typename TCell = Cell, typename TSimulation = Simulation<>>
void RunTest() {
  TSimulation simulation("displacement_op_test_RunTest");
  auto* rm = simulation.GetResourceManager();

  const auto* cells = rm->template Get<Cell>();

  // Cell 1
  TCell cell1;
  cell1.SetAdherence(0.3);
  cell1.SetDiameter(9);
  cell1.SetMass(1.4);
  cell1.SetPosition({0, 0, 0});
  // cell.SetTractorForce(tractor_force);
  rm->push_back(cell1);

  // Cell 2
  TCell cell2;
  cell2.SetAdherence(0.4);
  cell2.SetDiameter(11);
  cell2.SetMass(1.1);
  cell2.SetPosition({0, 5, 0});
  rm->push_back(cell2);

  simulation.GetGrid()->Initialize();

  // execute operation
  DisplacementOp<> op;
  rm->ApplyOnAllElements([&](auto&& sim_object, SoHandle) { op(sim_object); });

  // check results
  // cell 1
  auto final_position = (*cells)[0].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 2
  final_position = (*cells)[1].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0980452768658333, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 1
  auto final_tf = (*cells)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 2
  final_tf = (*cells)[1].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 1
  EXPECT_NEAR(0.3, (*cells)[0].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, (*cells)[0].GetMass(), abs_error<double>::value);
  // cell 2
  EXPECT_NEAR(0.4, (*cells)[1].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, (*cells)[1].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, (*cells)[1].GetMass(), abs_error<double>::value);
}

}  // namespace displacement_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_DISPLACEMENT_OP_TEST_H_
