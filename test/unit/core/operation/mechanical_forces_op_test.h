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
  EXPECT_NEAR(0, final_position[0], abs_error<real_t>::value);
  EXPECT_NEAR(-0.0494590384253325649715244643239, final_position[1],
              abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<real_t>::value);
  // cell 1
  final_position = final_cell1->GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<real_t>::value);
  EXPECT_NEAR(5.06220489350739084993450043382, final_position[1], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<real_t>::value);

  // check if tractor_force has been reset to zero
  // cell 0
  auto final_tf = final_cell0->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<real_t>::value);
  // cell 1
  final_tf = final_cell1->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<real_t>::value);

  // remaining fields should remain unchanged
  // cell 0
  EXPECT_NEAR(0.3, final_cell0->GetAdherence(), abs_error<real_t>::value);
  EXPECT_NEAR(9, final_cell0->GetDiameter(), abs_error<real_t>::value);
  EXPECT_NEAR(1.4, final_cell0->GetMass(), abs_error<real_t>::value);
  // cell 1
  EXPECT_NEAR(0.4, final_cell1->GetAdherence(), abs_error<real_t>::value);
  EXPECT_NEAR(11, final_cell1->GetDiameter(), abs_error<real_t>::value);
  EXPECT_NEAR(1.1, final_cell1->GetMass(), abs_error<real_t>::value);

  delete op;
}

inline void RunTest2(const std::string& environment) {
  auto set_param = [&](auto* param) { param->environment = environment; };
  Simulation simulation("mechanical_forces_op_test_RunTest", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  real_t space = 20;
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
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 0)->GetPosition(), {-0.136650834179647427449523566794, -0.136650834179647427449523566794, -0.136650834179647427449523566794});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 1)->GetPosition(), {19.9978236653235009314379819452, -0.141015306673735183678484988771, -0.141015306673735183678484988771});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 2)->GetPosition(), {40.1365944997387418724705928750, -0.13576125310507888313992208380, -0.13576125310507888313992208380});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 3)->GetPosition(), {-0.14020905712356290333724784449, 19.9970489222767306358984663823, -0.140984279029959657535431602871});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 4)->GetPosition(), {19.9977851373048793139637014758, 19.9947099058674891346242305069, -0.145277624139375911514663940516});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 5)->GetPosition(), {40.1401388185864714084643836652, 19.9970715249564464264518999951, -0.14007326379892509482257787044});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 6)->GetPosition(), {-0.13574486720051072606503005095, 40.1365627529316842850017320254, -0.135723902739067722287343804703});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 7)->GetPosition(), {19.9978515057522633535842662641, 40.1392617421792200453064082392, -0.140043337731872570780977206275});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 8)->GetPosition(), {40.1356651625421862223149400611, 40.1356589129132839546632435903, -0.134846375207706975815553682423});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 9)->GetPosition(), {-0.140178174501168932165811283971, -0.140182476858812768317650143102, 19.9962788234873976200766333855});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 10)->GetPosition(), {19.9977862308751720034084509786, -0.144471737809141948159807531113, 19.9939369302356357458986222506});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 11)->GetPosition(), {40.1401080571319587799692461333, -0.139281644960194857517683474124, 19.9963112337827742562800909788});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 12)->GetPosition(), {-0.143636884973078624410733274983, 19.9970003497588359473907353340, 19.9939216324133267598249705192});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 13)->GetPosition(), {19.9977509281001790744848931789, 19.9946123129189552912189860276, 19.9914755278624715229091673136});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 14)->GetPosition(), {40.1435533751884737842596265535, 19.9970230447662831556627447542, 19.9939839395317135001120125930});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 15)->GetPosition(), {-0.139264372505880738304090978511, 40.1400752445670986163454723481, 19.9963056313964950800704680887});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 16)->GetPosition(), {19.9978140032122536103189974908, 40.1426469666206381715941682442, 19.9939685327911046231094964962});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 17)->GetPosition(), {40.1391708994512008085005089423, 40.1391603959532489256165370155, 19.9963378247512290443404377838});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 18)->GetPosition(), {-0.135703460393067269278536235005, -0.135707572042464525088701330238, 40.1365270491528043523269029624});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 19)->GetPosition(), {19.9978526859614648754687984712, -0.140029334896406153010924586016, 40.1392258289244541720704728805});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 20)->GetPosition(), {40.1356238143556917587581200186, -0.13483030541778221502463362419, 40.1356238215903481357585777958});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 21)->GetPosition(), {-0.139216463603416178525167668456, 19.9970888470926410320886260158, 40.1391768764013402113023482055});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 22)->GetPosition(), {19.9978151330991056052596669472, 19.9947714402862555915866948948, 40.1417125181374631632487142307});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 23)->GetPosition(), {40.1391231591415114231657215943, 19.9971109750473197461909678817, 40.1382739675789673370998391272});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 24)->GetPosition(), {-0.134810200828153573232552824888, 40.1355886055729997034154133547, 40.1355781327590436272539263897});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 25)->GetPosition(), {19.9978797095407642059976814643, 40.1382410601235742697478311222, 40.1382260635760805291432435869});
  EXPECT_ARR_NEAR(rm->GetAgent(ref_uid + 26)->GetPosition(), {40.1347081112982584823528491696, 40.1346979778000455604454679675, 40.1346876663558284147859833527});

  // clang-format on

  delete mechanical_forces_op;
}

}  // namespace mechanical_forces_op_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_OPERATION_MECHANICAL_FORCES_OP_TEST_H_
