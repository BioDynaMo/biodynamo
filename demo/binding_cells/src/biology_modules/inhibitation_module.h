// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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

#include "agents/t_cell.h"
#include "core/behavior/behavior.h"

#include "Math/DistFunc.h"

namespace bdm {

/// Inhibits Monocytes from forming an immune synapse with T-Cells
struct Inhibitation : public Behavior {
  BDM_BEHAVIOR_HEADER(Inhibitation, Behavior, 1);

 public:
  Inhibitation() { AlwaysCopyToNew(); }

  Inhibitation(real s, real m) : sigma_(s), mu_(m) { AlwaysCopyToNew(); }

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    auto* other = event.existing_behavior;
    if (Inhibitation* gdbm = dynamic_cast<Inhibitation*>(other)) {
      sigma_ = gdbm->sigma_;
      mu_ = gdbm->mu_;
    } else {
      Log::Fatal("Inhibitation::EventConstructor",
                 "other was not of type Inhibitation");
    }
  }

  void Run(Agent* agent) override {
    if (auto* monocyte = static_cast<Monocyte*>(agent)) {
      // If this monocyte is already inhibited, we can prune this function
      if (monocyte->IsInhibited()) {
        return;
      }
      auto* rm = Simulation::GetActive()->GetResourceManager();
      auto* dgrid = rm->GetDiffusionGrid(0);
      real conc = dgrid->GetConcentration(monocyte->GetPosition());

      // With certain probability, depending on concentration value, we
      // inhibit the monocyte from forming immune synapses
      auto* r = Simulation::GetActive()->GetRandom();
      if (r->Uniform(0, 1) <
          ROOT::Math::normal_cdf(std::log(conc) / std::log(10), sigma_, mu_)) {
        monocyte->Inhibit();
      }
    }
  }

 private:
  real sigma_ = 1;
  real mu_ = 0;
};

}  // namespace bdm

#endif  // INHIBITATION_MODULE_H_
