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
#include "core/util/log.h"

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

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance() const { return new GrowDivide(); }

  void EventConstructor(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) override {
    BaseBiologyModule::EventConstructor(event, other, new_oid);
    if(GrowDivide* gdbm = dynamic_cast<GrowDivide*>(other)) {
      threshold_ = gdbm->threshold_;
      growth_rate_ = gdbm->growth_rate_;
    } else {
      Log::Fatal("GrowDivide::EventConstructor", "other was not of type GrowDivide");
    }
  }

  /// Default event handler (exising biology module won't be modified on
  /// any event)
  void EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2 = nullptr) override {
    BaseBiologyModule::EventHandler(event, other1, other2);
  }

  void Run(SimObject* so) override {
    if(Cell* cell = dynamic_cast<Cell*>(so)) {
      if (cell->GetDiameter() <= threshold_) {
        cell->ChangeVolume(growth_rate_);
      } else {
        cell->Divide();
      }
    } else {
      Log::Fatal("GrowDivide::Run", "SimObject is not a Cell");
    }
  }

 private:
  BDM_CLASS_DEF_NV(GrowDivide, 1);
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_
