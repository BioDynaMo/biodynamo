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

#ifndef CORE_BIOLOGY_MODULE_SECRETION_H_
#define CORE_BIOLOGY_MODULE_SECRETION_H_

#include "core/biology_module/biology_module.h"
#include "core/diffusion_grid.h"
#include "core/sim_object/cell.h"

namespace bdm {

/// Secrete substance at SimObject position
struct Secretion : public BaseBiologyModule {
  BDM_BM_HEADER(Secretion, BaseBiologyModule, 1);

  Secretion(DiffusionGrid* dgrid, double quantity = 1,
            std::initializer_list<EventId> copy_events = {gAllEventIds},
            std::initializer_list<EventId> remove_events = {})
      : BaseBiologyModule(copy_events, remove_events),
        dgrid_(dgrid),
        quantity_(quantity) {}

  Secretion(const Event& event, BaseBiologyModule* other, uint64_t new_uid)
      : BaseBiologyModule(event, other, new_uid) {
    dgrid_ = bdm_static_cast<Secretion*>(other)->dgrid_;
    quantity_ = bdm_static_cast<Secretion*>(other)->quantity_;
  }

  virtual ~Secretion() {}

  void Run(SimObject* so) override {
    auto& secretion_position = so->GetPosition();
    dgrid_->IncreaseConcentrationBy(secretion_position, quantity_);
  }

 private:
  DiffusionGrid* dgrid_ = nullptr;
  double quantity_ = 1;
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_SECRETION_H_
