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

#ifndef CORE_BEHAVIOR_GROW_DIVIDE_H_
#define CORE_BEHAVIOR_GROW_DIVIDE_H_

#include "core/behavior/behavior.h"
#include "core/event/cell_division_event.h"
#include "core/agent/cell.h"
#include "core/util/log.h"
#include "core/util/root.h"

namespace bdm {

/// This behavior grows the agent until the diameter reaches
/// the specified threshold and divides the object afterwards.
struct GrowDivide : public Behavior {
  BDM_BEHAVIOR_HEADER(GrowDivide, Behavior, 1);
  GrowDivide() : Behavior(gAllEventIds) {}
  GrowDivide(double threshold, double growth_rate,
             std::initializer_list<EventId> event_list)
      : Behavior(event_list),
        threshold_(threshold),
        growth_rate_(growth_rate) {}

  GrowDivide(const Event& event, Behavior* other, uint64_t new_oid = 0)
      : Behavior(event, other, new_oid) {
    if (GrowDivide* gd = dynamic_cast<GrowDivide*>(other)) {
      threshold_ = gd->threshold_;
      growth_rate_ = gd->growth_rate_;
    } else {
      Log::Fatal("GrowDivide::EventConstructor",
                 "other was not of type GrowDivide");
    }
  }

  /// Default event handler (exising behavior won't be modified on
  /// any event)

  void Run(Agent* agent) override {
    if (Cell* cell = dynamic_cast<Cell*>(agent)) {
      if (cell->GetDiameter() <= threshold_) {
        cell->ChangeVolume(growth_rate_);
      } else {
        cell->Divide();
      }
    } else {
      Log::Fatal("GrowDivide::Run", "Agent is not a Cell");
    }
  }

 private:
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // CORE_BEHAVIOR_GROW_DIVIDE_H_
