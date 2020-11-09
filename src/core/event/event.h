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

#ifndef CORE_EVENT_EVENT_H_
#define CORE_EVENT_EVENT_H_

#include <limits>
#include <mutex>
#include "core/util/log.h"
#include "core/container/inline_vector.h"

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

  static NewAgentEventUidGenerator* GetInstance() {
    static NewAgentEventUidGenerator kInstance;
    return &kInstance;
  }

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

  Agent* existing_agent;
  InlineVector<Agent*, 3> new_agents;
  Behavior* existing_behavior;
  InlineVector<Behavior*, 3> new_behaviors;
};

}  // namespace bdm

#endif  // CORE_EVENT_EVENT_H_
