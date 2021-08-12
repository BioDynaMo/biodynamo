// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef UNIT_CORE_OPERATION_MECHANICAL_FORCES_OP_TEST_H_
#define UNIT_CORE_OPERATION_MECHANICAL_FORCES_OP_TEST_H_

#include "core/agent/cell.h"
#include "core/operation/mechanical_forces_op.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace mechanical_forces_op_test_internal {
inline void RunTest(const std::string& environment) {
  auto set_param = [&](auto* param) { param->environment = environment; };
  Simulation simulation("mechanical_forces_op_test_RunTest", set_param);
  auto* rm = simulation.GetResourceManager();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  // cell 0
  Cell* cell0 = new Cell();
  cell0->SetAdherence(0.3);
  cell0->SetDiameter(9);
  cell0->SetMass(1.4);
  cell0->SetPosition({0, 0, 0});
  rm->AddAgent(cell0);

  // cell 1
  Cell* cell1 = new Cell();
  cell1->SetAdherence(0.4);
  cell1->SetDiameter(11);
  cell1->SetMass(1.1);
  cell1->SetPosition({0, 5, 0});
  rm->AddAgent(cell1);

  simulation.GetEnvironment()->Update();

  // execute operation
  auto* ctxt = simulation.GetExecutionContext();
  auto* op = NewOperation("mechanical forces");
  ctxt->Execute(rm->GetAgent(ref_uid), rm->GetAgentHandle(ref_uid), {op});
  ctxt->Execute(rm->GetAgent(ref_uid + 1), rm->GetAgentHandle(ref_uid + 1),
                {op});

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

inline void RunTest2(const std::string& environment) {
  auto set_param = [&](auto* param) { param->environment = environment; };
  Simulation simulation("mechanical_forces_op_test_RunTest", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  double space = 20;
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      for (size_t k = 0; k < 3; k++) {
        Cell* cell = new Cell({k * space, j * space, i * space});
        cell->SetDiameter(30);
        cell->SetAdherence(0.4);
        cell->SetMass(1.0);
        rm->AddAgent(cell);
      }
    }
  }

  env->Clear();
  env->Update();

  // Create operation
  auto* mechanical_forces_op = NewOperation("mechanical forces");

  // execute operation
  auto* ctxt = simulation.GetExecutionContext();

  for (uint64_t i = 0; i < 27; i++) {
    ctxt->Execute(rm->GetAgent(ref_uid + i), rm->GetAgentHandle(ref_uid + 1),
                  {mechanical_forces_op});
  }

  // clang-format off
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 0)->GetPosition(), {-0.20160966809506442, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 1)->GetPosition(), {19.996726567569418, -0.22266672163763598, -0.22266672163763598});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 2)->GetPosition(), {40.201498173927305, -0.19986951265938641, -0.19986951265938641});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 3)->GetPosition(), {-0.22107235667371566, 19.99536937981701, -0.22243298066161277});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 4)->GetPosition(), {19.996445694277536, 19.991084649064845, -0.24302072168423453});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 5)->GetPosition(), {40.220941141351886, 19.995420431150347, -0.22054684872504365});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 6)->GetPosition(), {-0.19982869510573351, 40.201437349791846, -0.19959785780667869});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 7)->GetPosition(), {19.99678197072755, 40.219229813316794, -0.22031963374181529});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 8)->GetPosition(), {40.199677082903456, 40.19967381378099, -0.19789135788064738});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 9)->GetPosition(), {-0.2208401632148318, -0.22084860029084805, 19.994023485540392});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 10)->GetPosition(), {19.996453518945092, -0.2413353410797266, 19.989660774799667});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 11)->GetPosition(), {40.220709535497228, -0.21898884436167229, 19.994096990730121});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 12)->GetPosition(), {-0.23959752549368377, 19.995004901057818, 19.989639428390195});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 13)->GetPosition(), {19.996198530091725, 19.99036483907971, 19.984534769039289});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 14)->GetPosition(), {40.239451672678122, 19.995058936152933, 19.989784932711256});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 15)->GetPosition(), {-0.21895170976356038, 40.220651199374309, 19.994102585380165});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 16)->GetPosition(), {19.996511705095408, 40.237587787915302, 19.989763131796483});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 17)->GetPosition(), {40.218778612717593, 40.218767244773701, 19.994175099676564});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 18)->GetPosition(), {-0.19955064743630646, -0.19955761561629054, 40.201369950961137});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 19)->GetPosition(), {19.99679050342159, -0.22028367546827032, 40.219170471740057});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 20)->GetPosition(), {40.199399442577274, -0.19785191118676104, 40.199607855810939});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 21)->GetPosition(), {-0.2186842147430082, 19.995465190535288, 40.218898427254011});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 22)->GetPosition(), {19.996519321027819, 19.991238784452595, 40.235673745336591});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 23)->GetPosition(), {40.218511916378013, 19.995514817696375, 40.217044778537549});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 24)->GetPosition(), {-0.19780590198157336, 40.19933398679354, 40.199323871317965});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 25)->GetPosition(), {19.996843796541615, 40.216799284843852, 40.216779234925141});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 26)->GetPosition(), {40.197616806612238, 40.197607143403182, 40.197597121203316});
  // clang-format on

  delete mechanical_forces_op;
}

}  // namespace mechanical_forces_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_MECHANICAL_FORCES_OP_TEST_H_
