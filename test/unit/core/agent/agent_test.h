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

#ifndef UNIT_CORE_AGENT_AGENT_TEST_H_
#define UNIT_CORE_AGENT_AGENT_TEST_H_

#include <gtest/gtest.h>
#include "core/agent/agent.h"
#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "unit/test_util/test_agent.h"

namespace bdm {
namespace agent_test_internal {

struct Growth : public Behavior {
  BDM_BEHAVIOR_HEADER(Growth, Behavior, 1);

  double growth_rate_ = 0.5;

  Growth() { CopyToNewIf({CellDivisionEvent::kUid}); }

  virtual ~Growth() = default;

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    auto* other = event.existing_behavior;
    if (auto* g = dynamic_cast<Growth*>(other)) {
      growth_rate_ = g->growth_rate_;
    } else {
      Log::Fatal("Growth::EventConstructor", "other was not of type Growth");
    }
  }

  void Run(Agent* t) override {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }
};

struct Movement : public Behavior {
  BDM_BEHAVIOR_HEADER(Movement, Behavior, 1);
  Double3 velocity_;

  Movement() : velocity_({{0, 0, 0}}) {
    RemoveFromExistingIf({CellDivisionEvent::kUid});
  }
  explicit Movement(const Double3& velocity) : velocity_(velocity) {
    RemoveFromExistingIf({CellDivisionEvent::kUid});
  }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    auto* other = event.existing_behavior;
    if (auto* m = dynamic_cast<Movement*>(other)) {
      velocity_ = m->velocity_;
    } else {
      Log::Fatal("Movement::EventConstructor",
                 "other was not of type Movement");
    }
  }

  void Run(Agent* agent) override {
    const auto& position = agent->GetPosition();
    agent->SetPosition(position + velocity_);
  }
};

/// This behavior removes itself the first time it is executed
struct Removal : public Behavior {
  BDM_BEHAVIOR_HEADER(Removal, Movement, 1);

  Removal() = default;
  virtual ~Removal() = default;

  void Run(Agent* agent) override { agent->RemoveBehavior(this); }
};

}  // namespace agent_test_internal

// -----------------------------------------------------------------------------
struct CaptureStaticness : public Behavior {
  BDM_BEHAVIOR_HEADER(CaptureStaticness, Behavior, 1);

  CaptureStaticness() = default;
  CaptureStaticness(std::unordered_map<AgentUid, bool>* static_agents_map)
      : static_agents_map_(static_agents_map) {}
  virtual ~CaptureStaticness() = default;

  void Run(Agent* agent) override {
#pragma omp critical
    (*static_agents_map_)[agent->GetUid()] = agent->IsStatic();
  }

 private:
  std::unordered_map<AgentUid, bool>* static_agents_map_;
};

#ifdef __ROOTCLING__
static AgentPointer<Agent> dummy_ptr;
#endif

}  // namespace bdm

#endif  // UNIT_CORE_AGENT_AGENT_TEST_H_
