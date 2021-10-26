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

#include "core/execution_context/copy_execution_context.h"
#include "core/agent/agent.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
void CopyExecutionContext::Use(Simulation* sim) {
  auto size = sim->GetAllExecCtxts().size();
  auto map = std::make_shared<
      typename InPlaceExecutionContext::ThreadSafeAgentUidMap>();
  std::vector<ExecutionContext*> exec_ctxts(size);
  auto agents = std::make_shared<std::vector<std::vector<Agent*>>>();
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < size; i++) {
    exec_ctxts[i] = new CopyExecutionContext(map, agents);
  }
  sim->SetAllExecCtxts(exec_ctxts);
}

// -----------------------------------------------------------------------------
CopyExecutionContext::CopyExecutionContext(
    const std::shared_ptr<ThreadSafeAgentUidMap>& map,
    std::shared_ptr<std::vector<std::vector<Agent*>>> agents)
    : InPlaceExecutionContext(map), agents_(agents) {
  auto* tinfo = ThreadInfo::GetInstance();
#pragma omp master
  agents_->resize(tinfo->GetNumaNodes());

  auto* param = Simulation::GetActive()->GetParam();
  if (param->execution_order == Param::ExecutionOrder::kForEachOpForEachAgent) {
    Log::Fatal("CopyExecutionContext",
               "CopyExecutionContext does not support Param::execution_order = "
               "Param::ExecutionOrder::kForEachOpForEachAgent");
  }
}

// -----------------------------------------------------------------------------
CopyExecutionContext::~CopyExecutionContext() {}

// -----------------------------------------------------------------------------
void CopyExecutionContext::SetupIterationAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  InPlaceExecutionContext::SetupIterationAll(all_exec_ctxts);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (uint64_t n = 0; n < agents_->size(); ++n) {
    agents_->at(n).reserve(rm->GetAgentVectorCapacity(n));
    agents_->at(n).resize(rm->GetNumAgents(n));
  }

  auto* scheduler = Simulation::GetActive()->GetScheduler();
  if (scheduler->GetAgentFilters().size() != 0) {
    Log::Fatal("CopyExecutionContext",
               "CopyExecutionContext does not support simulations with agent "
               "filters yet (see Scheduler::SetAgentFilters)");
  }
}

// -----------------------------------------------------------------------------
void CopyExecutionContext::TearDownAgentOpsAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  auto del = L2F([](Agent* a) { delete a; });
  rm->ForEachAgentParallel(del);
  rm->SwapAgents(agents_.get());
}

// -----------------------------------------------------------------------------
void CopyExecutionContext::Execute(Agent* agent, AgentHandle ah,
                                   const std::vector<Operation*>& operations) {
  auto* copy = agent->NewCopy();
  InPlaceExecutionContext::Execute(copy, ah, operations);
  assert(ah.GetNumaNode() < agents_->size());
  assert(ah.GetElementIdx() < agents_->at(ah.GetNumaNode()).size());
  (*agents_.get())[ah.GetNumaNode()][ah.GetElementIdx()] = copy;
}

}  // namespace experimental
}  // namespace bdm
