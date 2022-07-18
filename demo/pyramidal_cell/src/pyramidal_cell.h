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
#ifndef PYRAMIDAL_CELL_H_
#define PYRAMIDAL_CELL_H_

#include <fstream>
#include <iostream>
#include "biodynamo.h"
#include "neuroscience/neuroscience.h"

namespace bdm {

enum Substances { kApical, kBasal };

struct ApicalDendriteGrowth : public Behavior {
  BDM_BEHAVIOR_HEADER(ApicalDendriteGrowth, Behavior, 1);
  ApicalDendriteGrowth() { AlwaysCopyToNew(); }
  virtual ~ApicalDendriteGrowth() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    can_branch_ = false;
  }

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* rm = sim->GetResourceManager();

    if (!init_) {
      dg_guide_ = rm->GetDiffusionGrid(kApical);
      init_ = true;
    }

    auto* dendrite = bdm_static_cast<NeuriteElement*>(agent);
    if (dendrite->GetDiameter() > 0.575) {
      Real3 gradient;
      dg_guide_->GetGradient(dendrite->GetPosition(), &gradient);

      real_t gradient_weight = 0.06;
      real_t randomness_weight = 0.3;
      real_t old_direction_weight = 4;

      auto random_axis = random->template UniformArray<3>(-1, 1);
      auto old_direction = dendrite->GetSpringAxis() * old_direction_weight;
      auto grad_direction = gradient * gradient_weight;
      auto random_direction = random_axis * randomness_weight;

      Real3 new_step_direction =
          old_direction + random_direction + grad_direction;

      dendrite->ElongateTerminalEnd(100, new_step_direction);
      dendrite->SetDiameter(dendrite->GetDiameter() - 0.00071);

      if (can_branch_ && dendrite->IsTerminal() &&
          dendrite->GetDiameter() > 0.55 && random->Uniform() < 0.038) {
        auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
        Real3 branch_direction =
            Math::Perp3(dendrite->GetUnitaryAxisDirectionVector() + rand_noise,
                        random->Uniform(0, 1)) +
            dendrite->GetSpringAxis();
        auto* dendrite_2 = dendrite->Branch(branch_direction);
        dendrite_2->SetDiameter(0.65);
      }
    }
  }

 private:
  bool init_ = false;
  bool can_branch_ = true;
  DiffusionGrid* dg_guide_ = nullptr;
};

struct BasalDendriteGrowth : public Behavior {
  BDM_BEHAVIOR_HEADER(BasalDendriteGrowth, Behavior, 1);
  BasalDendriteGrowth() { AlwaysCopyToNew(); }
  virtual ~BasalDendriteGrowth() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* rm = sim->GetResourceManager();

    if (!init_) {
      dg_guide_ = rm->GetDiffusionGrid(kBasal);
      init_ = true;
    }

    auto* dendrite = bdm_static_cast<NeuriteElement*>(agent);
    if (dendrite->IsTerminal() && dendrite->GetDiameter() > 0.75) {
      Real3 gradient;
      dg_guide_->GetGradient(dendrite->GetPosition(), &gradient);

      real_t gradient_weight = 0.03;
      real_t randomness_weight = 0.4;
      real_t old_direction_weight = 6;

      auto random_axis = random->template UniformArray<3>(-1, 1);
      auto old_direction = dendrite->GetSpringAxis() * old_direction_weight;
      auto grad_direction = gradient * gradient_weight;
      auto random_direction = random_axis * randomness_weight;

      Real3 new_step_direction =
          old_direction + random_direction + grad_direction;

      dendrite->ElongateTerminalEnd(50, new_step_direction);
      dendrite->SetDiameter(dendrite->GetDiameter() - 0.00085);

      if (random->Uniform() < 0.006) {
        dendrite->Bifurcate();
      }
    }
  }

 private:
  bool init_ = false;
  DiffusionGrid* dg_guide_ = nullptr;
};

inline void AddInitialNeuron(const Real3& position) {
  auto* soma = new neuroscience::NeuronSoma(position);
  soma->SetDiameter(10);
  Simulation::GetActive()->GetResourceManager()->AddAgent(soma);

  auto* apical_dendrite = soma->ExtendNewNeurite({0, 0, 1});
  auto* basal_dendrite1 = soma->ExtendNewNeurite({0, 0, -1});
  auto* basal_dendrite2 = soma->ExtendNewNeurite({0, 0.6, -0.8});
  auto* basal_dendrite3 = soma->ExtendNewNeurite({0.3, -0.6, -0.8});

  apical_dendrite->AddBehavior(new ApicalDendriteGrowth());
  basal_dendrite1->AddBehavior(new BasalDendriteGrowth());
  basal_dendrite2->AddBehavior(new BasalDendriteGrowth());
  basal_dendrite3->AddBehavior(new BasalDendriteGrowth());
}

/// Create and initialize substances for neurite attraction
inline void CreateExtracellularSubstances(const Param* p) {
  using MI = ModelInitializer;
  MI::DefineSubstance(kApical, "substance_apical", 0, 0, p->max_bound / 80);
  MI::DefineSubstance(kBasal, "substance_basal", 0, 0, p->max_bound / 80);
  // initialize substance with gaussian distribution
  auto a_initializer = GaussianBand(p->max_bound, 200, Axis::kZAxis);
  auto b_initializer = GaussianBand(p->min_bound, 200, Axis::kZAxis);
  MI::InitializeSubstance(kApical, a_initializer);
  MI::InitializeSubstance(kBasal, b_initializer);
}

/// Saves the morphology of the neuron in the SWC file format.
inline void SaveNeuronMorphology(Simulation& sim) {
  auto* rm = sim.GetResourceManager();
  rm->ForEachAgent([&](Agent* agent) {
    auto* soma = dynamic_cast<neuroscience::NeuronSoma*>(agent);
    if (soma != nullptr) {
      std::ofstream myfile;
      myfile.open(sim.GetOutputDir() + "/neuron.swc");
      soma->PrintSWC(myfile);
    }
  });
}

inline int Simulate(int argc, const char** argv) {
  neuroscience::InitModule();
  Simulation simulation(argc, argv);
  AddInitialNeuron({150, 150, 0});
  CreateExtracellularSubstances(simulation.GetParam());
  simulation.GetScheduler()->Simulate(500);
  SaveNeuronMorphology(simulation);
  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // PYRAMIDAL_CELL_H_
