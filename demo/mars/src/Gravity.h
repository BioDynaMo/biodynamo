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
#ifndef GRAVITY_H_
#define GRAVITY_H_

#include "CelestialObjects.h"
#include "biodynamo.h"

namespace bdm {
namespace astrophysics {

// Gravitational constant in m^3/kg/s^2
inline real_t gG = 6.674e-11;

// set units of G constant
inline void SetUnits(char units = 'O', int times = 1) {
  gG = 6.674e-11;
  switch (units) {
    case 'k':
      gG *= pow(times * 1e-3, 3);
      break;
    case 'M':
      gG *= pow(times * 1e-6, 3);
      break;
    case 'G':
      gG *= pow(times * 1e-9, 3);
      break;
    default:
      gG *= pow(times, 3);
      break;
  }
}

// a behavior that applies Newtons Law for gravity and calculates the
// displacement for any celestial object
class Gravity : public Behavior {
  Real3 acceleration_ = {0, 0, 0};
  real_t dt_ = Simulation::GetActive()->GetParam()->simulation_time_step;

 public:
  Behavior* New() const override { return new Gravity(); }

  Behavior* NewCopy() const override { return new Gravity(*this); }

  void Run(Agent* agent) override {
    CelestialObject* celestial = dynamic_cast<CelestialObject*>(agent);

    Real3 celestial_pos = celestial->GetPosition();

    auto* functor = new GravityFunctor(celestial, gG);
    // for each agent calculate acceleration
    Simulation::GetActive()->GetResourceManager()->ForEachAgentParallel(
        *functor);

    dt_ = Simulation::GetActive()->GetParam()->simulation_time_step;
    Real3 velocity = celestial->GetVelocity();
    Real3 acceleration = functor->acceleration;
    Real3 position;

    // explicit Euler's method
    velocity += acceleration * dt_;
    position = celestial_pos + velocity * dt_;

    delete functor;

    celestial->SetVelocity(velocity);
    celestial->SetPosition(position);
  }

  class GravityFunctor : public Functor<void, Agent*, AgentHandle> {
   public:
    CelestialObject* celestial;
    real_t g;
    Real3 acceleration;

    GravityFunctor(CelestialObject* celestial, real_t g)
        : celestial(celestial), g(g), acceleration({0, 0, 0}) {}

    // override the function that overloads () operator to
    // calculate acceleration based on given agent
    void operator()(Agent* other_agent, AgentHandle) override {
      CelestialObject* other_celestial =
          dynamic_cast<CelestialObject*>(other_agent);

      if (!other_celestial || other_celestial == celestial) {
        return;
      }

      Real3 dir = other_celestial->GetPosition() - celestial->GetPosition();
      real_t magnitude = dir.Norm();

      dir.Normalize();
      real_t m1 = celestial->GetMass();
      real_t m2 = other_celestial->GetMass();
      real_t f = g * m1 * m2 / (magnitude * magnitude);

      acceleration += (f / m1) * dir;
    };
  };
};
}  // namespace astrophysics

}  // namespace bdm

#endif  // GRAVITY_H_
