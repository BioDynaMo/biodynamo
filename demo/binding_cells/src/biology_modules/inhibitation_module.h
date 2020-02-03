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
#ifndef INHIBITATION_MODULE_H_
#define INHIBITATION_MODULE_H_

#include "biodynamo.h"
#include "simulation_objects/t_cell.h"

namespace bdm {

/// Inhibits Monocytes from forming an immune synapse with T-Cells
struct Inhibitation : public BaseBiologyModule {
 public:
  Inhibitation() : BaseBiologyModule(gAllEventIds) {}

  Inhibitation(double c, double p)
      : BaseBiologyModule(gAllEventIds), conc_threshold_(c), probability_(p) {}

  Inhibitation(const Event& event, BaseBiologyModule* other,
               uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (Inhibitation* gdbm = dynamic_cast<Inhibitation*>(other)) {
      conc_threshold_ = gdbm->conc_threshold_;
      probability_ = gdbm->probability_;
    } else {
      Log::Fatal("Inhibitation::EventConstructor",
                 "other was not of type Inhibitation");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new Inhibitation(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override {
    return new Inhibitation(*this);
  }

  void Run(SimObject* so) override {
    if (auto* monocyte = dynamic_cast<Monocyte*>(so)) {
      // If this monocyte is already inhibited, we can prune this function
      if (monocyte->IsInhibited()) {
        return;
      }
      auto* rm = Simulation::GetActive()->GetResourceManager();
      auto* dgrid = rm->GetDiffusionGrid(0);
      double conc = dgrid->GetConcentration(monocyte->GetPosition());

      // With certain probability, depending on concentration value, we
      // inhibit the monocyte from forming an immune synapse
      auto* r = Simulation::GetActive()->GetRandom();
      if ((conc >= conc_threshold_) &&
          (std::abs(r->Uniform()) < probability_)) {
        monocyte->Inhibit();
      }
    }
  }

 private:
  double conc_threshold_ = 1;
  double probability_ = 0.05;
};

}  // namespace bdm

#endif  // INHIBITATION_MODULE_H_
