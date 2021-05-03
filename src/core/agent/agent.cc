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

#include "core/agent/new_agent_event.h"
#include "core/behavior/behavior.h"
#include "core/environment/environment.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"

namespace bdm {

Agent::Agent() {
  uid_ = Simulation::GetActive()->GetAgentUidGenerator()->GenerateUid();
}

Agent::Agent(TRootIOCtor* io_ctor) {}

Agent::Agent(const Agent& other)
    : uid_(other.uid_),
      box_idx_(other.box_idx_),
      run_behavior_loop_idx_(other.run_behavior_loop_idx_),
      propagate_staticness_neighborhood_(
          other.propagate_staticness_neighborhood_),
      is_static_next_ts_(other.is_static_next_ts_) {
  for (auto* behavior : other.behaviors_) {
    behaviors_.push_back(behavior->NewCopy());
  }
}

Agent::~Agent() {
  for (auto* el : behaviors_) {
    delete el;
  }
}

void Agent::Initialize(const NewAgentEvent& event) {
  box_idx_ = event.existing_agent->GetBoxIdx();
  // copy behaviors_ to me
  InitializeBehaviors(event);
}

void Agent::Update(const NewAgentEvent& event) {
  // Run displacement if a new agent has been created with an event.
  SetPropagateStaticness();
  UpdateBehaviors(event);
}

struct SetStaticnessForEachNeighbor : public Functor<void, Agent*, double> {
  Agent* agent_;
  explicit SetStaticnessForEachNeighbor(Agent* agent) : agent_(agent) {}

  void operator()(Agent* neighbor, double squared_distance) override {
    double distance = agent_->GetDiameter() + neighbor->GetDiameter();
    if (squared_distance < distance * distance) {
      neighbor->SetStaticnessNextTimestep(false);
    }
  }
};

void Agent::PropagateStaticness() {
  if (!Simulation::GetActive()->GetParam()->detect_static_agents) {
    is_static_next_ts_ = false;
    return;
  }

  if (!propagate_staticness_neighborhood_) {
    return;
  }
  propagate_staticness_neighborhood_ = false;
  is_static_next_ts_ = false;
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  SetStaticnessForEachNeighbor for_each(this);
  ctxt->ForEachNeighbor(for_each, *this);
}

void Agent::RunDiscretization() {}

void Agent::AssignNewUid() {
  uid_ = Simulation::GetActive()->GetAgentUidGenerator()->GenerateUid();
}

const AgentUid& Agent::GetUid() const { return uid_; }

uint32_t Agent::GetBoxIdx() const { return box_idx_; }

void Agent::SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

// ---------------------------------------------------------------------------
// Behaviors

void Agent::AddBehavior(Behavior* behavior) { behaviors_.push_back(behavior); }

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

const InlineVector<Behavior*, 2>& Agent::GetAllBehaviors() const {
  return behaviors_;
}
// ---------------------------------------------------------------------------

void Agent::RemoveFromSimulation() const {
  Simulation::GetActive()->GetExecutionContext()->RemoveAgent(uid_);
}

void Agent::InitializeBehaviors(const NewAgentEvent& event) {
  const auto& existing_agent_behaviors = event.existing_agent->behaviors_;
  event.new_behaviors.clear();
  uint64_t cnt = 0;
  for (auto* behavior : existing_agent_behaviors) {
    if (behavior->WillBeCopied(event.GetUid())) {
      event.new_behaviors.clear();
      // collect new behaviors from other new agents
      for (auto* nagent : event.new_agents) {
        event.new_behaviors.push_back(nagent->behaviors_[cnt]);
      }
      event.existing_behavior = behavior;
      auto* new_behavior = behavior->New();
      new_behavior->Initialize(event);
      behaviors_.push_back(new_behavior);
      cnt++;
    }
  }
}

void Agent::UpdateBehaviors(const NewAgentEvent& event) {
  // call event handler for behaviors
  uint64_t cnt = 0;
  for (auto* behavior : behaviors_) {
    bool copied = behavior->WillBeCopied(event.GetUid());
    if (!behavior->WillBeRemoved(event.GetUid())) {
      event.new_behaviors.clear();
      if (copied) {
        for (auto* new_agent : event.new_agents) {
          auto* new_behavior = new_agent->behaviors_[cnt];
          event.new_behaviors.push_back(new_behavior);
        }
      }
      behavior->Update(event);
    }
    cnt += copied ? 1 : 0;
  }

  // remove behaviors from current
  for (auto it = behaviors_.begin(); it != behaviors_.end();) {
    auto* behavior = *it;
    if (behavior->WillBeRemoved(event.GetUid())) {
      delete *it;
      it = behaviors_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace bdm
