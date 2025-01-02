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
class CelestialObject : public Agent {
  BDM_AGENT_HEADER(CelestialObject, Agent, 1);

 public:
  // constructors
  CelestialObject() : diameter_(1.0), density_(1.0) { UpdateVolume(); }

  explicit CelestialObject(real_t diameter)
      : diameter_(diameter), density_(1.0) {
    UpdateVolume();
  }

  explicit CelestialObject(const Real3& position)
      : position_(position), diameter_(1.0), density_(1.0) {
    UpdateVolume();
  }

  // default destructor
  ~CelestialObject() override = default;

  Real3 GetVelocity() const { return velocity_; }

  real_t GetDiameter() const override { return diameter_; }

  real_t GetMass() const { return density_ * volume_; }

  real_t GetDensity() const { return density_; }

  const Real3& GetPosition() const override { return position_; }

  real_t GetVolume() const { return volume_; }

  Shape GetShape() const override { return Shape::kSphere; }

  void SetVelocity(const Real3& vel) { velocity_ = vel; }

  void SetDiameter(real_t diameter) override {
    if (diameter > diameter_) {
      SetPropagateStaticness();
    }
    diameter_ = diameter;
    UpdateVolume();
  }

  void SetVolume(real_t volume) {
    volume_ = volume;
    UpdateDiameter();
  }

  void SetMass(real_t mass) { SetDensity(mass / volume_); }

  void SetDensity(real_t density) {
    if (density > density_) {
      SetPropagateStaticness();
    }
    density_ = density;
  }

  void SetPosition(const Real3& position) override {
    position_ = position;
    SetPropagateStaticness();
  }

  void ChangeVolume(real_t speed) {
    // scaling for integration step
    auto* param = Simulation::GetActive()->GetParam();
    real_t delta = speed * param->simulation_time_step;
    volume_ += delta;
    if (volume_ < real_t(5.2359877E-7)) {
      volume_ = real_t(5.2359877E-7);
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    real_t diameter = std::cbrt(volume_ * 6 / Math::kPi);
    if (diameter > diameter_) {
      Base::SetPropagateStaticness();
    }
    diameter_ = diameter;
  }

  void UpdateVolume() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    volume_ = Math::kPi / real_t(6) * std::pow(diameter_, 3);
  }

  void UpdatePosition(const Real3& delta) {
    position_ += delta;
    SetPropagateStaticness();
  }

  // empty unused functions
  void ApplyDisplacement(const Real3& displacement) override{};

  Real3 CalculateDisplacement(const InteractionForce* force,
                              real_t squared_radius, real_t dt) override {
    return Real3{0, 0, 0};
  }

 private:
  Real3 position_ = {0, 0, 0};
  real_t diameter_ = 0;
  real_t volume_ = 0;
  real_t density_ = 0;
  Real3 velocity_ = {0, 0, 0};
};

// Star subclass
class Star : public CelestialObject {
  BDM_AGENT_HEADER(Star, CelestialObject, 1);

 public:
  Star() : CelestialObject() {}

  explicit Star(real_t diameter) : CelestialObject(diameter) {}

  explicit Star(const Real3 position) : CelestialObject(position) {}

  ~Star() override = default;
};

// Planet subclass
class Planet : public CelestialObject {
  BDM_AGENT_HEADER(Planet, CelestialObject, 1);

 public:
  Planet() : CelestialObject() {}

  explicit Planet(real_t diameter) : CelestialObject(diameter) {}

  explicit Planet(const Real3 position) : CelestialObject(position) {}

  explicit Planet(const Star* main_star, real_t diameter, real_t distance,
                  real_t initial_speed) {
    Real3 pos = main_star->GetPosition();
    pos[0] += distance;
    Real3 initial_velocity = {0, initial_speed, 0};
    this->SetVelocity(main_star->GetVelocity() + initial_velocity);
    this->SetPosition(pos);
    this->SetDiameter(diameter);
  }

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
