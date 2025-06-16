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
#ifndef NEWTONSLAWTEST_H_
#define NEWTONSLAWTEST_H_

#include "CelestialObjects.h"
#include "Gravity.h"
#include "biodynamo.h"

namespace bdm {
namespace astrophysics {

inline int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);
  auto* rm = simulation.GetResourceManager();

  // create 4 celestial objects. Apply mass initial position and velocity
  auto* object1 = new CelestialObject(10);
  object1->SetMass(10e+16);
  object1->SetPosition({50, 0, 0});
  object1->SetVelocity({100, 100, 100});

  auto* object2 = new CelestialObject(5);
  object2->SetMass(10e+16);
  object2->SetPosition({-50, 0, 0});
  object2->SetVelocity({100, -100, -100});

  auto* object3 = new CelestialObject(5);
  object3->SetMass(5e+16);
  object3->SetPosition({-50, -50, 0});
  object3->SetVelocity({200, -50, -20});

  auto* object4 = new CelestialObject(50);
  object4->SetMass(10e+17);
  object4->SetPosition({100, 100, 0});
  object4->SetVelocity({-20, 50, 20});

  // Add gravity behaviour to all agents
  object1->AddBehavior(new Gravity);
  object2->AddBehavior(new Gravity);
  object3->AddBehavior(new Gravity);
  object4->AddBehavior(new Gravity);

  // Add agents to simulation
  rm->AddAgent(object1);
  rm->AddAgent(object2);
  rm->AddAgent(object3);
  rm->AddAgent(object4);

  simulation.GetScheduler()->Simulate(200);
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace astrophysics
}  // namespace bdm
#endif  // NEWTONSLAWTEST_H_
