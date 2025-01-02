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

#include "biodynamo.h"
#include "CelestialObjects.h"
#include "Gravity.h"
#include "DataReader.h"

namespace bdm {
  namespace astrophysics{
    
    Real3 RotateVector(const Real3& Vec, real_t th, char Axis);

    inline int Simulate(int argc, const char** argv) {

      Simulation simulation(argc, argv);
      auto* rm = simulation.GetResourceManager();

      std::vector<ObjectData> data;
      std::string filename = "planets.txt";

      loadDataFromFile(filename, data);

      constexpr real_t diameter_scaling_factor = 10;
      SetUnits('M');

      //create sun
      Star* Sun = new Star(1392.7 * diameter_scaling_factor); 
      Sun->SetPosition({0, 0, 0});
      Sun->SetMass(1988400e+24);
      Sun->AddBehavior(new Gravity);
      rm->AddAgent(Sun);

      //read data from .txt file and create the planets
      for (auto object : data)
      {
        auto* planet = new Planet(object.diameter * diameter_scaling_factor);
        planet->SetMass(object.mass * 1e+24);
        planet->SetPosition({object.distanceFromParent, 0, 0});
        Real3 V = RotateVector({0, object.orbitalVelocity, 0}, object.angle, 'x');
        planet->SetVelocity(V);
        planet->AddBehavior(new Gravity);
        rm->AddAgent(planet);
      }

      simulation.GetScheduler()->Simulate(900);
      return 0;
    }

  }  // namespace astrophysics
}   // namespace bdm
#endif  //SolarSystem.h
