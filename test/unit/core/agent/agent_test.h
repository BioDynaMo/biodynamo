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

#ifndef UNIT_CORE_AGENT_AGENT_TEST_H_
#define UNIT_CORE_AGENT_AGENT_TEST_H_

#include <gtest/gtest.h>
#include "core/behavior/behavior.h"
#include "core/agent/cell.h"
#include "core/agent/agent.h"
#include "unit/test_util/test_agent.h"

namespace bdm {
namespace agent_test_internal {

struct GrowthModule : public Behavior {
  double growth_rate_ = 0.5;

  GrowthModule() : Behavior(CellDivisionEvent::kEventId) {}

  GrowthModule(const Event& event, Behavior* other,
               uint64_t new_oid = 0)
      : Behavior(event, other, new_oid) {
    if (GrowthModule* gbm = dynamic_cast<GrowthModule*>(other)) {
      growth_rate_ = gbm->growth_rate_;
    } else {
      Log::Fatal("GrowthModule::EventConstructor",
                 "other was not of type GrowthModule");
    }
  }

  virtual ~GrowthModule() {}

  Behavior* GetInstance(const Event& event, Behavior* other,
                                 uint64_t new_oid = 0) const override {
    return new GrowthModule(event, other, new_oid);
  }
  Behavior* GetCopy() const override {
    return new GrowthModule(*this);
  }

  /// Default event handler (exising behavior won't be modified on
  /// any event)
  void EventHandler(const Event& event, Behavior* other1,
                    Behavior* other2 = nullptr) override {
    Behavior::EventHandler(event, other1, other2);
  }

  void Run(Agent* t) override {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  BDM_CLASS_DEF_OVERRIDE(GrowthModule, 1);
};

struct MovementModule : public Behavior {
  Double3 velocity_;

  MovementModule()
      : Behavior(0, CellDivisionEvent::kEventId),
        velocity_({{0, 0, 0}}) {}
  explicit MovementModule(const Double3& velocity)
      : Behavior(0, CellDivisionEvent::kEventId),
        velocity_(velocity) {}

  MovementModule(const Event& event, Behavior* other,
                 uint64_t new_oid = 0)
      : Behavior(event, other, new_oid) {
    if (MovementModule* mbm = dynamic_cast<MovementModule*>(other)) {
      velocity_ = mbm->velocity_;
    } else {
      Log::Fatal("MovementModule::EventConstructor",
                 "other was not of type MovementModule");
    }
  }

  /// Create a new instance of this object using the default constructor.
  Behavior* GetInstance(const Event& event, Behavior* other,
                                 uint64_t new_oid = 0) const override {
    return new MovementModule(event, other, new_oid);
  }
  Behavior* GetCopy() const override {
    return new MovementModule(*this);
  }

  /// Default event handler
  void EventHandler(const Event& event, Behavior* other1,
                    Behavior* other2 = nullptr) override {
    Behavior::EventHandler(event, other1, other2);
  }

  void Run(Agent* agent) override {
    const auto& position = agent->GetPosition();
    agent->SetPosition(position + velocity_);
  }

  BDM_CLASS_DEF_OVERRIDE(MovementModule, 1);
};

/// This behavior removes itself the first time it is executed
struct RemoveModule : public Behavior {
  RemoveModule() {}
  RemoveModule(const Event& event, Behavior* other,
               uint64_t new_oid = 0)
      : Behavior(event, other, new_oid) {}

  Behavior* GetInstance(const Event& event, Behavior* other,
                                 uint64_t new_oid = 0) const override {
    return new RemoveModule(event, other, new_oid);
  }
  Behavior* GetCopy() const override {
    return new RemoveModule(*this);
  }

  void Run(Agent* agent) override {
    agent->RemoveBehavior(this);
  }

  BDM_CLASS_DEF_OVERRIDE(RemoveModule, 1);
};

}  // namespace agent_test_internal

#ifdef __ROOTCLING__
static AgentPointer<Agent> dummy_ptr;
#endif

}  // namespace bdm

#endif  // UNIT_CORE_AGENT_AGENT_TEST_H_
