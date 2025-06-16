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
#ifndef SOLARSYSTEM_H_
#define SOLARSYSTEM_H_

#include "CelestialObjects.h"
#include "DataReader.h"
#include "Gravity.h"
#include "biodynamo.h"

namespace bdm {
namespace astrophysics {

Real3 RotateVector(const Real3& vec, real_t th, char axis);

inline int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);
  auto* rm = simulation.GetResourceManager();

  std::vector<ObjectData> data;
  std::string filename = "planets.txt";

  LoadDataFromFile(filename, data);

  SetUnits('G');

  // create sun
  Star* sun = new Star(109);
  sun->SetPosition({0, 0, 0});
  sun->SetMass(1988400e+24);
  sun->AddBehavior(new Gravity);
  rm->AddAgent(sun);

  // read data from .txt file and create the planets
  for (auto object : data) {
    auto* planet = new Planet(object.diameter);
    planet->SetMass(object.mass * 1e+24);
    planet->SetPosition({object.distance_from_parent, 0, 0});
    Real3 v = RotateVector({0, object.orbital_velocity, 0}, object.angle, 'x');
    planet->SetVelocity(v);
    planet->AddBehavior(new Gravity);
    rm->AddAgent(planet);
  }

  simulation.GetScheduler()->Simulate(900);
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace astrophysics
}  // namespace bdm
#endif  // SolarSystem.h
