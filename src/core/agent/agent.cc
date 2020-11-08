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

#include "core/agent/agent.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/behavior/behavior.h"
#include "core/environment/environment.h"
#include "core/event/event.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"

namespace bdm {

Agent::Agent() {
  uid_ = Simulation::GetActive()->GetAgentUidGenerator()->NewAgentUid();
}

Agent::Agent(const Event& event, Agent* other, uint64_t new_oid)
    : Agent() {
  box_idx_ = other->GetBoxIdx();
  // behaviors
  auto* other_behaviors = &(other->behaviors_);
  // copy behaviors_ to me
  CopyBehaviors(event, other_behaviors);
}

Agent::Agent(TRootIOCtor* io_ctor) {}

Agent::Agent(const Agent& other)
    : uid_(other.uid_),
      box_idx_(other.box_idx_),
      run_behavior_loop_idx_(other.run_behavior_loop_idx_),
      run_displacement_for_all_next_ts_(
          other.run_displacement_for_all_next_ts_),
      run_displacement_next_ts_(other.run_displacement_next_ts_) {
  for (auto* behavior : other.behaviors_) {
    behaviors_.push_back(behavior->GetCopy());
  }
}

Agent::~Agent() {
  for (auto* el : behaviors_) {
    delete el;
  }
}

struct SetRunDisplacementForEachNeighbor
    : public Functor<void, const Agent*, double> {
  Agent* agent_;
  SetRunDisplacementForEachNeighbor(Agent* agent) : agent_(agent) {}

  void operator()(const Agent* neighbor, double squared_distance) override {
    double distance = agent_->GetDiameter() + neighbor->GetDiameter();
    if (squared_distance < distance * distance) {
      neighbor->SetRunDisplacementNextTimestep(true);
    }
  }
};

void Agent::DistributeRunDisplacementInfo() {
  if (!Simulation::GetActive()->GetParam()->detect_static_agents) {
    run_displacement_next_ts_ = true;
    return;
  }

  if (!run_displacement_for_all_next_ts_) {
    return;
  }
  run_displacement_for_all_next_ts_ = false;
  run_displacement_next_ts_ = true;
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  SetRunDisplacementForEachNeighbor for_each(this);
  ctxt->ForEachNeighbor(for_each, *this);
}

void Agent::RunDiscretization() {}

void Agent::AssignNewUid() {
  uid_ = Simulation::GetActive()->GetAgentUidGenerator()->NewAgentUid();
}

const AgentUid& Agent::GetUid() const { return uid_; }

uint32_t Agent::GetBoxIdx() const { return box_idx_; }

void Agent::SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

// ---------------------------------------------------------------------------
// Behaviors

void Agent::AddBehavior(Behavior* behavior) {
  behaviors_.push_back(behavior);
}

void Agent::RemoveBehavior(const Behavior* behavior) {
  for (unsigned int i = 0; i < behaviors_.size(); i++) {
    if (behaviors_[i] == behavior) {
      delete behavior;
      behaviors_.erase(behaviors_.begin() + i);
      // if behavior was before or at the current run_behavior_loop_idx_,
      // correct it by subtracting one.
      run_behavior_loop_idx_ -= i > run_behavior_loop_idx_ ? 0 : 1;
    }
  }
}

void Agent::RunBehaviors() {
  for (run_behavior_loop_idx_ = 0; run_behavior_loop_idx_ < behaviors_.size();
       ++run_behavior_loop_idx_) {
    auto* behavior = behaviors_[run_behavior_loop_idx_];
    behavior->Run(this);
  }
}

const InlineVector<Behavior*, 2>& Agent::GetAllBehaviors()
    const {
  return behaviors_;
}
// ---------------------------------------------------------------------------

void Agent::RemoveFromSimulation() const {
  Simulation::GetActive()->GetExecutionContext()->RemoveFromSimulation(uid_);
}

void Agent::EventHandler(const Event& event, Agent* other1,
                             Agent* other2) {
  // Run displacement if a new agent has been created with an event.
  SetRunDisplacementForAllNextTs();
  // call event handler for behaviors
  auto* left_behaviors = other1 == nullptr ? nullptr : &(other1->behaviors_);
  auto* right_behaviors = other2 == nullptr ? nullptr : &(other2->behaviors_);
  BehaviorEventHandler(event, left_behaviors, right_behaviors);
}

void Agent::CopyBehaviors(const Event& event,
                                   decltype(behaviors_) * other) {
  for (auto* behavior : *other) {
    if (behavior->Copy(event.GetId())) {
      auto* new_behavior = behavior->GetInstance(event, behavior);
      behaviors_.push_back(new_behavior);
    }
  }
}

void Agent::BehaviorEventHandler(const Event& event,
                                          decltype(behaviors_) * other1,
                                          decltype(behaviors_) * other2) {
  // call event handler for behaviors
  uint64_t cnt = 0;
  for (auto* behavior : behaviors_) {
    bool copy = behavior->Copy(event.GetId());
    if (!behavior->Remove(event.GetId())) {
      if (copy) {
        auto* other1_behavior = other1 != nullptr ? (*other1)[cnt] : nullptr;
        auto* other2_behavior = other2 != nullptr ? (*other2)[cnt] : nullptr;
        behavior->EventHandler(event, other1_behavior, other2_behavior);
      } else {
        behavior->EventHandler(event, nullptr, nullptr);
      }
    }
    cnt += copy ? 1 : 0;
  }

  // remove behaviors from current
  for (auto it = behaviors_.begin(); it != behaviors_.end();) {
    auto* behavior = *it;
    if (behavior->Remove(event.GetId())) {
      delete *it;
      it = behaviors_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace bdm
