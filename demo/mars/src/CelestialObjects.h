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
#ifndef CELESTIALOBJECTS_H_
#define CELESTIALOBJECTS_H_

#include "biodynamo.h"

namespace bdm {
namespace astrophysics {

// creating the custom agent that
// defines celestial objects
class CelestialObject : public Cell {
  BDM_AGENT_HEADER(CelestialObject, Cell, 1);

 public:
  // constructors
  CelestialObject() { UpdateVolume(); }

  explicit CelestialObject(real_t diameter) {
    this->SetDiameter(diameter);
    UpdateVolume();
  }

  explicit CelestialObject(const Real3& position) {
    this->SetPosition(position);
    UpdateVolume();
  }

  // default destructor
  ~CelestialObject() override = default;

  Real3 GetVelocity() const { return velocity_; }

  void SetVelocity(const Real3& vel) { velocity_ = vel; }

 private:
  Real3 velocity_ = {0, 0, 0};
};

// Planet subclass
class Planet : public CelestialObject {
  BDM_AGENT_HEADER(Planet, CelestialObject, 1);

 public:
  Planet() : CelestialObject() {}

  explicit Planet(real_t diameter) : CelestialObject(diameter) {}

  explicit Planet(const Real3 position) : CelestialObject(position) {}

  ~Planet() override = default;
};

// Satellite subclass
class Satellite : public CelestialObject {
  BDM_AGENT_HEADER(Satellite, CelestialObject, 1);

 public:
  Satellite() : CelestialObject() {}

  explicit Satellite(real_t diameter) : CelestialObject(diameter) {}

  explicit Satellite(const Real3 position) : CelestialObject(position) {}

  explicit Satellite(const Planet* main_planet, real_t diameter,
                     real_t distance, real_t initial_speed) {
    Real3 pos = main_planet->GetPosition();
    pos[0] += distance;
    Real3 initial_velocity = {0, initial_speed, 0};
    this->SetVelocity(main_planet->GetVelocity() + initial_velocity);

    this->SetPosition(pos);
    this->SetDiameter(diameter);
  }

  ~Satellite() override = default;
};

}  // namespace astrophysics
}  // namespace bdm

#endif  // CELESTIALOBJECT_H_
