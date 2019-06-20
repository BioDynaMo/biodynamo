// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_DEFAULT_FORCE_H_
#define CORE_DEFAULT_FORCE_H_

#include <array>

#include "core/container/math_array.h"

namespace bdm {

class SimObject;

class DefaultForce {
 public:
  DefaultForce() {}
  ~DefaultForce() {}
  DefaultForce(const DefaultForce&) = delete;
  DefaultForce& operator=(const DefaultForce&) = delete;

  Double4 GetForce(const SimObject* lhs, const SimObject* rhs);

 private:
  void ForceBetweenSpheres(const SimObject* sphere_lhs,
                           const SimObject* sphere_rhs, Double3* result) const;

  void ForceOnACylinderFromASphere(const SimObject* cylinder,
                                   const SimObject* sphere,
                                   Double4* result) const;

  void ForceOnASphereFromACylinder(const SimObject* sphere,
                                   const SimObject* cylinder,
                                   Double3* result) const;

  void ForceBetweenCylinders(const SimObject* cylinder1,
                             const SimObject* cylinder2, Double4* result) const;

  Double4 ComputeForceOfASphereOnASphere(const Double3& c1, double r1,
                                         const Double3& c2, double r2) const;
};

}  // namespace bdm

#endif  // CORE_DEFAULT_FORCE_H_
