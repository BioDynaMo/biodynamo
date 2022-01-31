// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_AGENT_NEW_AGENT_EVENT_H_
#define CORE_AGENT_NEW_AGENT_EVENT_H_

#include <limits>
#include <mutex>
#include "core/container/inline_vector.h"
#include "core/util/log.h"

namespace bdm {

/// NewAgentEventUid is used inside behaviors to determine if a behavior
/// should be copied if a new agent is created.
/// Possible events are cell division, neurite branching, ...\n
/// NewAgentEventUid invariant: the number of bits set to 1 must be 1.
using NewAgentEventUid = uint64_t;

class Agent;
class Behavior;

/// This class generates unique ids for behavior events satisfying the
/// NewAgentEventUid invariant. Thread safe.
class NewAgentEventUidGenerator {
 public:
  NewAgentEventUidGenerator(const NewAgentEventUidGenerator&) = delete;
  NewAgentEventUidGenerator& operator=(const NewAgentEventUidGenerator&) =
      delete;

  static NewAgentEventUidGenerator* GetInstance();

  NewAgentEventUid GenerateUid() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    constexpr uint64_t kOne = 1;
    if (counter_ == 64) {
      Log::Fatal("NewAgentEventUidGenerator",
                 "BioDynaMo only supports 64 unique NewAgentEventUids."
                 " You requested a 65th one.");
    }
    return kOne << counter_++;
  }

 private:
  NewAgentEventUidGenerator() {}
  std::recursive_mutex mutex_;
  uint64_t counter_ = 0;
};

struct NewAgentEvent {
  virtual ~NewAgentEvent() {}

  virtual NewAgentEventUid GetUid() const = 0;

  /// Pointer to agent that triggered the NewAgentEvent.\n
  /// e.g. for CellDivisionEvent it is the mother cell and
  /// for NewNeuriteExtensionEvent it is the NeuronSoma.
  mutable Agent* existing_agent;
  /// Vector of new agents that have been created during
  /// this NewAgentEvent. Agents are added to this vector
  /// aft er the call to `Initialize` completed.
  mutable InlineVector<Agent*, 3> new_agents;
  /// Similarly, to existing_agent, existing_behavior behavior
  /// points to the currently processed behavior of the
  /// existing agent.
  mutable Behavior* existing_behavior;
  /// Vector of behaviors that have been copied to new agents.
  /// The index in new_behaviors corresponds to the index in
  /// new_agents. That means, that new_behaviors[0] is the
  /// copy of existing_behavior for new_agent[0].
  mutable InlineVector<Behavior*, 3> new_behaviors;
};

}  // namespace bdm

#endif  // CORE_AGENT_NEW_AGENT_EVENT_H_
