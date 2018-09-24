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

#ifndef BIOLOGY_MODULE_GROW_DIVIDE_H_
#define BIOLOGY_MODULE_GROW_DIVIDE_H_

#include <Rtypes.h>
#include "biology_module_util.h"
#include "cell.h"
#include "event/cell_division_event.h"

namespace bdm {

/// This biology module grows the simulation object until the diameter reaches
/// the specified threshold and divides the object afterwards.
struct GrowDivide : public BaseBiologyModule {
  GrowDivide() : BaseBiologyModule(gAllEventIds) {}
  GrowDivide(double threshold, double growth_rate,
             std::initializer_list<EventId> event_list)
      : BaseBiologyModule(event_list),
        threshold_(threshold),
        growth_rate_(growth_rate) {}

  /// Default event constructor
  GrowDivide(const Event& event, GrowDivide* other, uint64_t new_oid = 0) {
    threshold_ = other->threshold_;
    growth_rate_ = other->growth_rate_;
  }

  BaseBiologyModule* New(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) const {
    return new GrowDivide(event, dynamic_cast<GrowDivide*>(other), new_oid);
  }

  /// Default event handler (exising biology module won't be modified on
  /// any event)
  void EventHandler(const Event&, BaseBiologyModule* other) override {}

  void Run(SimulationObject* so) override {
    if(Cell* cell = dynamic_cast<Cell*>(so)) {
      if (cell->GetDiameter() <= threshold_) {
        cell->ChangeVolume(growth_rate_);
      } else {
        cell->Divide();
      }
    } else {
      Fatal("GrowDivide", "SimulationObject must be a Cell!");
    }
  }

 private:
  // ClassDefNV(GrowDivide, 1);
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_GROW_DIVIDE_H_
