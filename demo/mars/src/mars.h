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
#ifndef MARS_H_
#define MARS_H_

#include "CelestialObjects.h"
#include "Gravity.h"
#include "biodynamo.h"

namespace bdm {
namespace astrophysics {

// Rotate a vector by a specified angle along a given axis
Real3 RotateVector(const Real3& vec, real_t th, char axis);

inline int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);
  auto* rm = simulation.GetResourceManager();

  SetUnits('k');  // Simulation units in kilometers.

  // Create Mars with distance from the Sun and initial orbital velocity.
  auto* mars = new Planet(6779);
  mars->SetPosition({0, 0, 0});
  mars->SetMass(0.64171e+24);
  mars->AddBehavior(new Gravity);
  rm->AddAgent(mars);

  // Create Phobos
  auto* phobos = new Satellite(22.4);  // Phobos diameter in kilometers
  phobos->SetMass(1.0659e+16);         // Phobos mass in kg
  phobos->SetPosition({mars->GetPosition()[0] + 9376, mars->GetPosition()[1],
                       mars->GetPosition()[2]});  // Position relative to Mars
  Real3 v_phobos =
      mars->GetVelocity() +
      RotateVector({0, 2.138, 0}, 0, 'x');  // Phobos velocity relative to Mars
  phobos->SetVelocity(v_phobos);
  phobos->AddBehavior(new Gravity);
  rm->AddAgent(phobos);

  // Create Deimos
  auto* deimos = new Satellite(12.4);
  deimos->SetMass(1.48e+15);
  deimos->SetPosition({mars->GetPosition()[0] + 23459, mars->GetPosition()[1],
                       mars->GetPosition()[2]});  // Position relative to Mars
  Real3 v_deimos =
      mars->GetVelocity() +
      RotateVector({0, 1.351, 0}, 0, 'x');  // Deimos velocity relative to Mars
  deimos->SetVelocity(v_deimos);
  deimos->AddBehavior(new Gravity);
  rm->AddAgent(deimos);

  simulation.GetScheduler()->Simulate(500);
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace astrophysics
}  // namespace bdm

#endif  // MARS_H_
