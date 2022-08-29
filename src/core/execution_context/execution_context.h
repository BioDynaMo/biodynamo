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

#ifndef CORE_EXECUTION_CONTEXT_EXECUTION_CONTEXT_H_
#define CORE_EXECUTION_CONTEXT_EXECUTION_CONTEXT_H_

#include <utility>
#include <vector>

#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/container/math_array.h"
#include "core/functor.h"
#include "core/operation/operation.h"

namespace bdm {

class Agent;

class ExecutionContext {
 public:
  virtual ~ExecutionContext() = default;

  /// This function is called before all agent operations are executed.\n
  /// This function is not thread-safe.
  /// NB: Invalidates references and pointers to agents.
  virtual void SetupAgentOpsAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) = 0;

  /// This function is called after all agent operations were executed.\n
  /// This function is not thread-safe. \n
  /// NB: Invalidates references and pointers to agents.
  virtual void TearDownAgentOpsAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) = 0;

  /// This function is called at the beginning of each iteration to setup all
  /// execution contexts.
  /// This function is not thread-safe.
  /// NB: Invalidates references and pointers to agents.
  virtual void SetupIterationAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) = 0;

  /// This function is called at the end of each iteration to tear down all
  /// execution contexts.
  /// This function is not thread-safe. \n
  /// NB: Invalidates references and pointers to agents.
  virtual void TearDownIterationAll(
      const std::vector<ExecutionContext*>& all_exec_ctxts) = 0;

  /// Execute a series of operations on an agent in the order given
  /// in the argument
  virtual void Execute(Agent* agent, AgentHandle ah,
                       const std::vector<Operation*>& operations) = 0;

  /// Applies the lambda `lambda` for each neighbor of the given `query`
  /// agent within the given `criteria`. Does not support caching.
  virtual void ForEachNeighbor(Functor<void, Agent*>& lambda,
                               const Agent& query, void* criteria) = 0;

  /// Applies the lambda `lambda` for each neighbor of the given `query`
  /// agent within the given search radius `sqrt(squared_radius)`
  virtual void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                               const Agent& query, real_t squared_radius) = 0;

  /// Applies the lambda `lambda` for each neighbor of the given
  /// `query_position` within the given search radius `sqrt(squared_radius)`
  virtual void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                               const Real3& query_position,
                               real_t squared_radius) = 0;

  virtual void AddAgent(Agent* new_agent) = 0;

  virtual void RemoveAgent(const AgentUid& uid) = 0;

  virtual Agent* GetAgent(const AgentUid& uid) = 0;

  virtual const Agent* GetConstAgent(const AgentUid& uid) = 0;
};

}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_EXECUTION_CONTEXT_H_
