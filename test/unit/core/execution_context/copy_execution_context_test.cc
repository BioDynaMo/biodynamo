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

#include <gtest/gtest.h>

#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/execution_context/copy_execution_context.h"
#include "core/operation/operation_registry.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
struct CopyExecCtxtOp : public AgentOperationImpl {
  BDM_OP_HEADER(CopyExecCtxtOp);

  void operator()(Agent* agent) override { agent->SetDiameter(345); }
};

BDM_REGISTER_OP(CopyExecCtxtOp, "CopyExecCtxtOp", kCpu);

// -----------------------------------------------------------------------------
TEST(CopyExecutionContext, Execute) {
  Simulation sim(TEST_NAME);

  // set CopyExecutionContext
  CopyExecutionContext::Use(&sim);

  auto* ctxt = sim.GetExecutionContext();
  auto* rm = sim.GetResourceManager();

  auto* cell = new Cell(123);
  auto uid = cell->GetUid();
  ctxt->AddAgent(cell);

  EXPECT_EQ(0u, rm->GetNumAgents());

  ctxt->SetupIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(1u, rm->GetNumAgents());

  auto* op = NewOperation("CopyExecCtxtOp");
  std::vector<Operation*> operations = {op};
  ctxt->Execute(cell, AgentHandle(0, 0), operations);

  // In the rm there should still be the unchanged cell
  EXPECT_NEAR(123., rm->GetAgent(uid)->GetDiameter(), abs_error<real>::value);

  // This call should commit the changes from the exec ctxt to the rm
  ctxt->TearDownAgentOpsAll(sim.GetAllExecCtxts());

  EXPECT_NEAR(345., rm->GetAgent(uid)->GetDiameter(), abs_error<real>::value);
  EXPECT_EQ(1u, rm->GetNumAgents());

  delete op;
}

}  // namespace experimental
}  // namespace bdm
