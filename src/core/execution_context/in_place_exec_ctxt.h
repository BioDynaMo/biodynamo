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

#ifndef CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
#define CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "core/container/agent_uid_map.h"
#include "core/functor.h"
#include "core/operation/operation.h"
#include "core/agent/agent_uid.h"
#include "core/util/spinlock.h"
#include "core/util/thread_info.h"

namespace bdm {

class Agent;

/// This execution context updates agents in place. \n
/// Let's assume we have two sim objects `A, B` in our simulation that we want
/// to update to the next timestep `A*, B*`. If we have one thread it will first
/// update `A` and afterwards `B` and write the updates directly to the same
/// data structure. Therefore, before we start updating `B` the array looks
/// like this: `A*, B`. `B` already observes the updated `A`. \n
/// Operations in method `Execute` are executed in order given by the user.
/// Subsequent operations observe the changes of earlier operations.\n
/// In-place updates can lead to race conditions if agents not only
/// modify themselves, but also neighbors. Therefore, a protection mechanism has
/// been added. \see `Param::thread_safety_mechanism_`
/// New sim objects will only be visible at the next iteration. \n
/// Also removal of a sim object happens at the end of each iteration.
class InPlaceExecutionContext {
 public:
  struct ThreadSafeAgentUidMap {
    using value_type = std::pair<Agent*, uint64_t>;
    ThreadSafeAgentUidMap();
    ~ThreadSafeAgentUidMap();

    void Insert(const AgentUid& uid, const value_type& value);
    const value_type& operator[](const AgentUid& key);
    uint64_t Size() const;
    void Resize(uint64_t new_size);
    void RemoveOldCopies();

    using Map = AgentUidMap<value_type>;
    Spinlock lock_;
    Spinlock next_lock_;
    Map* map_;
    Map* next_;
    std::vector<Map*> previous_maps_;
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

  /// Execute a series of operations on a agent in the order given
  /// in the argument
  void Execute(Agent* agent, const std::vector<Operation*>& operations);

  void push_back(Agent* new_so);  // NOLINT

  void ForEachNeighbor(Functor<void, const Agent*, double>& lambda,
                       const Agent& query);

  /// Forwards the call to `Grid::ForEachNeighborWithinRadius`
  void ForEachNeighborWithinRadius(
      Functor<void, const Agent*, double>& lambda, const Agent& query,
      double squared_radius);

  Agent* GetAgent(const AgentUid& uid);

  const Agent* GetConstAgent(const AgentUid& uid);

  void RemoveFromSimulation(const AgentUid& uid);

 private:
  /// Lookup table AgentUid -> AgentPointer for new created sim objects
  std::shared_ptr<ThreadSafeAgentUidMap> new_agent_map_;

  ThreadInfo* tinfo_;

  /// Contains unique ids of sim objects that will be removed at the end of each
  /// iteration.
  std::vector<AgentUid> remove_;
  std::vector<Spinlock*> locks;

  /// Pointer to new sim objects
  std::vector<Agent*> new_agents_;

  /// prevent race conditions for cached Agents
  std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;

  std::vector<std::pair<const Agent*, double>> neighbor_cache_;
};

}  // namespace bdm

#endif  // CORE_EXECUTION_CONTEXT_IN_PLACE_EXEC_CTXT_H_
