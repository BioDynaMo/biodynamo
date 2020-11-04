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

#ifndef CORE_BIOLOGY_MODULE_CHEMOTAXIS_H_
#define CORE_BIOLOGY_MODULE_CHEMOTAXIS_H_

#include "core/biology_module/biology_module.h"
#include "core/diffusion_grid.h"
#include "core/agent/cell.h"

namespace bdm {

/// Move cells along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  BDM_BM_HEADER(Chemotaxis, BaseBiologyModule, 1);

  Chemotaxis(DiffusionGrid* dgrid, double speed,
             std::initializer_list<EventId> copy_events = {gAllEventIds},
             std::initializer_list<EventId> remove_events = {})
      : BaseBiologyModule(copy_events, remove_events),
        dgrid_(dgrid),
        speed_(speed) {}

  Chemotaxis(const Event& event, BaseBiologyModule* other, uint64_t new_uid)
      : BaseBiologyModule(event, other, new_uid) {
    dgrid_ = bdm_static_cast<Chemotaxis*>(other)->dgrid_;
    speed_ = bdm_static_cast<Chemotaxis*>(other)->speed_;
  }

  virtual ~Chemotaxis() {}

  void Run(Agent* agent) override {
    auto* cell = bdm_static_cast<Cell*>(agent);
    auto& position = cell->GetPosition();
    Double3 gradient;
    dgrid_->GetGradient(position, &gradient);  // returns normalized gradient
    cell->UpdatePosition(gradient * speed_);
  }

 private:
  DiffusionGrid* dgrid_ = nullptr;
  double speed_;
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_CHEMOTAXIS_H_
