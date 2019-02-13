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

#include "core/biology_module/grow_divide.h"
#include "core/event/cell_division_event.h"
#include "core/util/log.h"
#include "core/sim_object/cell.h"

namespace bdm {

  GrowDivide::GrowDivide() : BaseBiologyModule(gAllEventIds) {}
  GrowDivide::GrowDivide(double threshold, double growth_rate,
             std::initializer_list<EventId> event_list)
      : BaseBiologyModule(event_list),
        threshold_(threshold),
        growth_rate_(growth_rate) {}

  GrowDivide::GrowDivide(const Event& event, BaseBiologyModule* other, uint64_t new_oid) : BaseBiologyModule(event, other, new_oid) {
    if(GrowDivide* gdbm = dynamic_cast<GrowDivide*>(other)) {
      threshold_ = gdbm->threshold_;
      growth_rate_ = gdbm->growth_rate_;
    } else {
      Log::Fatal("GrowDivide::EventConstructor", "other was not of type GrowDivide");
    }
  }

  GrowDivide::~GrowDivide() {}

  BaseBiologyModule* GrowDivide::GetInstance(const Event& event, BaseBiologyModule* other, uint64_t new_oid) const  {
    return new GrowDivide(event, other, new_oid);
  }

  BaseBiologyModule* GrowDivide::GetCopy() const { return new GrowDivide(*this); }

  void GrowDivide::EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2)  {
    BaseBiologyModule::EventHandler(event, other1, other2);
  }

  void GrowDivide::Run(SimObject* so)  {
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

}  // namespace bdm
