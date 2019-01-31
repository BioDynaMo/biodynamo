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

#ifndef CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_
#define CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_

#include "core/biology_module/biology_module.h"
#include "core/event/cell_division_event.h"
#include "core/util/root.h"

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
  template <typename TEvent, typename TBm>
  GrowDivide(const TEvent& event, TBm* other, uint64_t new_oid = 0) {
    threshold_ = other->threshold_;
    growth_rate_ = other->growth_rate_;
  }

  /// Default event handler (exising biology module won't be modified on
  /// any event)
  template <typename TEvent, typename... TBms>
  void EventHandler(const TEvent&, TBms*...) {}

  template <typename T>
  void Run(T* cell) {
    if (cell->GetDiameter() <= threshold_) {
      cell->ChangeVolume(growth_rate_);
    } else {
      cell->Divide();
    }
  }

 private:
  BDM_CLASS_DEF_NV(GrowDivide, 1);
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_
