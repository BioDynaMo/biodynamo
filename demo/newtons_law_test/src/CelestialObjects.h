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

}  // namespace astrophysics
}  // namespace bdm

#endif  // CELESTIALOBJECT_H_
