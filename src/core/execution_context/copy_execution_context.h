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

#ifndef CORE_EXECUTION_CONTEXT_COPY_EXECUTION_CONTEXT_H_
#define CORE_EXECUTION_CONTEXT_COPY_EXECUTION_CONTEXT_H_

#include "core/execution_context/in_place_exec_ctxt.h"

namespace bdm {

class Simulation;

namespace experimental {

/// This execution context derives from `InPlaceExecutionContext` and replaces
/// the logic when agent updates will be visible to other agents.
/// The remaining implementation is the same as in `InPlaceExecutionContext`.
/// The `CopyExecutionContext` creates and updates a copy of an agent.
/// The changes to this copy are commited at the end of the iteration. \n
/// Thus, all agents see the same agent state if they read attributes from their
/// neighbors.
/// The value of the neighbor attributes will be from the last iteration. \n
/// NB: This execution context does *not* support neighbor modification,
/// `Param::ExecutionOrder::kForEachOpForEachAgent`, and agent filter
/// `Scheduler::SetAgentFilters`.
class CopyExecutionContext : public InPlaceExecutionContext {
 public:
  /// Use the CopyExecutionContext for simulation `sim`.
  static void Use(Simulation* sim);

  explicit CopyExecutionContext(
      const std::shared_ptr<ThreadSafeAgentUidMap>& map,
      std::shared_ptr<std::vector<std::vector<Agent*>>> agents);

  virtual ~CopyExecutionContext();

  void SetupIterationAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) override;

  void TearDownAgentOpsAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) override;

  void Execute(Agent* agent, AgentHandle ah,
               const std::vector<Operation*>& operations) override;

 protected:
  /// Pointer container for all agents shared between all
  /// CopyExecutionContext instances of a simulation.
  std::shared_ptr<std::vector<std::vector<Agent*>>> agents_;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_COPY_EXECUTION_CONTEXT_H_
