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

#ifndef CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
#define CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_

#include <atomic>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/container/agent_uid_map.h"
#include "core/functor.h"
#include "core/operation/operation.h"
#include "core/util/spinlock.h"
#include "core/util/thread_info.h"

namespace bdm {

class Agent;

/// This execution context updates agents in place. \n
/// Let's assume we have two agents `A, B` in our simulation that we want
/// to update to the next timestep `A*, B*`. If we have one thread it will first
/// update `A` and afterwards `B` and write the updates directly to the same
/// data structure. Therefore, before we start updating `B` the array looks
/// like this: `A*, B`. `B` already observes the updated `A`. \n
/// Operations in method `Execute` are executed in order given by the user.
/// Subsequent operations observe the changes of earlier operations.\n
/// In-place updates can lead to race conditions if agents not only
/// modify themselves, but also neighbors. Therefore, a protection mechanism has
/// been added. \see `Param::thread_safety_mechanism`
/// New agents will only be visible at the next iteration. \n
/// Also removal of an agent happens at the end of each iteration.
class InPlaceExecutionContext {
 public:
  struct ThreadSafeAgentUidMap {
    using value_type = Agent*;
    using Batch = std::vector<value_type>;
    ThreadSafeAgentUidMap();
    ~ThreadSafeAgentUidMap();

    void Insert(const AgentUid& uid, const value_type& value);
    const value_type& operator[](const AgentUid& key);
    uint64_t Size() const;
    void Resize(uint64_t new_size);
    void DeleteOldCopies();

    Spinlock lock_;
    constexpr static uint64_t kBatchSize = 10240;
    uint64_t num_batches_ = 0;
    std::atomic<Batch**> batches_;
    std::vector<Batch**> old_copies_;
  };

  explicit InPlaceExecutionContext(
      const std::shared_ptr<ThreadSafeAgentUidMap>& map);

  virtual ~InPlaceExecutionContext();

  /// This function is called at the beginning of each iteration to setup all
  /// execution contexts.
  /// This function is not thread-safe.
  /// NB: Invalidates references and pointers to agents.
  void SetupIterationAll(
      const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const;

  /// This function is called at the end of each iteration to tear down all
  /// execution contexts.
  /// This function is not thread-safe. \n
  /// NB: Invalidates references and pointers to agents.
  void TearDownIterationAll(
      const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const;

  /// Execute a series of operations on an agent in the order given
  /// in the argument
  void Execute(Agent* agent, const std::vector<Operation*>& operations);

  void ForEachNeighbor(Functor<void, const Agent*, double>& lambda,
                       const Agent& query);

  /// calls lambdas with agents that are within sqrt(squared_radius)
  /// from query
  void ForEachNeighbor(Functor<void, const Agent*, double>& lambda,
                       const Agent& query, double squared_radius);

  void AddAgent(Agent* new_agent);

  void RemoveAgent(const AgentUid& uid);

  Agent* GetAgent(const AgentUid& uid);

  const Agent* GetConstAgent(const AgentUid& uid);

 private:
  /// Lookup table AgentUid -> AgentPointer for new created agents
  std::shared_ptr<ThreadSafeAgentUidMap> new_agent_map_;

  ThreadInfo* tinfo_;

  /// Contains unique ids of agents that will be removed at the end of each
  /// iteration. AgentUids are separated by numa node.
  std::vector<AgentUid> remove_;
  std::vector<AgentUid> critical_region_;
  std::vector<AgentUid> critical_region_2_;
  std::vector<Spinlock*> locks_;

  /// Pointer to new agents
  std::vector<Agent*> new_agents_;

  /// prevent race conditions for cached Agents
  std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;

  std::vector<std::pair<const Agent*, double>> neighbor_cache_;
};

}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
