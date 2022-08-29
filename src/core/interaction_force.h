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

#ifndef CORE_INTERACTION_FORCE_H_
#define CORE_INTERACTION_FORCE_H_

#include <array>

#include "core/container/math_array.h"

namespace bdm {

class Agent;

class InteractionForce {
 public:
  InteractionForce() = default;
  virtual ~InteractionForce() = default;

  virtual Real4 Calculate(const Agent* lhs, const Agent* rhs) const;
  virtual InteractionForce* NewCopy() const {
    return new InteractionForce(*this);
  }

 private:
  void ForceBetweenSpheres(const Agent* sphere_lhs, const Agent* sphere_rhs,
                           Real3* result) const;

  void ForceOnACylinderFromASphere(const Agent* cylinder, const Agent* sphere,
                                   Real4* result) const;

  void ForceOnASphereFromACylinder(const Agent* sphere, const Agent* cylinder,
                                   Real3* result) const;

  void ForceBetweenCylinders(const Agent* cylinder1, const Agent* cylinder2,
                             Real4* result) const;

  Real4 ComputeForceOfASphereOnASphere(const Real3& c1, real_t r1,
                                       const Real3& c2, real_t r2) const;
};

}  // namespace bdm

#endif  // CORE_INTERACTION_FORCE_H_
