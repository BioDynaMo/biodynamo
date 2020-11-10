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

#ifndef CORE_BEHAVIOR_GROWTH_DIVISION_H_
#define CORE_BEHAVIOR_GROWTH_DIVISION_H_

#include "core/agent/cell.h"
#include "core/agent/cell_division_event.h"
#include "core/behavior/behavior.h"
#include "core/util/log.h"
#include "core/util/root.h"

namespace bdm {

/// This behavior grows the agent until the diameter reaches
/// the specified threshold and divides the object afterwards.
class GrowthDivision : public Behavior {
  BDM_BEHAVIOR_HEADER(GrowthDivision, Behavior, 1);

 public:
  GrowthDivision() { AlwaysCopyToNew(); }
  GrowthDivision(double threshold, double growth_rate)
      : threshold_(threshold), growth_rate_(growth_rate) {}

  virtual ~GrowthDivision() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    auto* other = event.existing_behavior;
    if (auto* gd = dynamic_cast<GrowthDivision*>(other)) {
      threshold_ = gd->threshold_;
      growth_rate_ = gd->growth_rate_;
    } else {
      Log::Fatal("GrowthDivision::Initialize",
                 "event.existing_behavior was not of type GrowthDivision");
    }
  }

  /// Default event handler (exising behavior won't be modified on
  /// any event)

  void Run(Agent* agent) override {
    if (auto* cell = dynamic_cast<Cell*>(agent)) {
      if (cell->GetDiameter() <= threshold_) {
        cell->ChangeVolume(growth_rate_);
      } else {
        cell->Divide();
      }
    } else {
      Log::Fatal("GrowthDivision::Run", "Agent is not a Cell");
    }
  }

 private:
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_GROWTH_DIVISION_H_
