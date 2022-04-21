// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef MY_BEHAVIOR_H_
#define MY_BEHAVIOR_H_

#include "biodynamo.h"
#include "core/behavior/behavior.h"
#include "my_agent.h"

namespace bdm {

/// Behavior that allows agents to sense the continuum value and stores the
/// value in its member variable.
struct RetrieveContinuumValue : public Behavior {
  BDM_BEHAVIOR_HEADER(RetrieveContinuumValue, Behavior, 1);

  RetrieveContinuumValue() {}
  virtual ~RetrieveContinuumValue() {}

  void Run(Agent* a) override {
    auto* sim = Simulation::GetActive();

    if (!continuum_model_) {
      // Get the continuum model in the first iteration.
      auto* rm = sim->GetResourceManager();
      auto* cm = rm->GetContinuumModel(0);
      continuum_model_ = dynamic_cast<ScalarField*>(cm);
      if (!continuum_model_) {
        Log::Fatal("Continuum model not found.");
      }
    }

    // Cast the agent to the right type.
    auto* cra = dynamic_cast<ContinuumRetrieverAgent*>(a);
    if (cra) {
      // Get the continuum value at the agent's position and update the agent's
      // member variable.
      cra->SetMyContinuumValue(continuum_model_->GetValue(cra->GetPosition()));
    }
  }

 private:
  ScalarField* continuum_model_ = nullptr;
};

}  // namespace bdm

#endif  // MY_BEHAVIOR_H_
