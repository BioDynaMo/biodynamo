#ifndef DIFFUSION_MODULES_H_
#define DIFFUSION_MODULES_H_

#include "biodynamo.h"

namespace bdm {

enum Substances { kKalium };
struct Chemotaxis : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(Chemotaxis, BaseBiologyModule, 1);
 public:
  Chemotaxis() : BaseBiologyModule(gAllEventIds) {}

  void Run(SimObject* so) override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      const auto& position = so->GetPosition();
      Double3 gradient;
      kDg->GetGradient(position, &gradient);
      gradient *= 0.5;
      cell->UpdatePosition(gradient);
    }
  }
};

struct KaliumSecretion : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(KaliumSecretion, BaseBiologyModule, 1);
 public:
  KaliumSecretion() : BaseBiologyModule() {}

  void Run(SimObject* so) override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);
    double amount = 4;
    kDg->IncreaseConcentrationBy(so->GetPosition(), amount);
  }
};

}
#endif
