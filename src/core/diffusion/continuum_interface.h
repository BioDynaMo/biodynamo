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

#ifndef CONTINUUM_INTERFACE_H_
#define CONTINUUM_INTERFACE_H_

#include <array>
#include <limits>
#include <string>
#include "core/container/math_array.h"

namespace bdm {

class Continuum {
 public:
  Continuum() = default;
  explicit Continuum(const TRootIOCtor *) {}
  virtual ~Continuum() = default;

  void IntegrateTimeAsynchronously(double dt);

  virtual void Initialize() = 0;
  virtual void Update() = 0;
  virtual void Step(double dt) = 0;

  int GetContinuumId() const { return continuum_id_; }
  void SetContinuumId(int id) { continuum_id_ = id; }
  const std::string &GetContinuumName() const { return continuum_name_; }
  void SetContinuumName(const std::string &name) { continuum_name_ = name; }
  double GetSimulatedTime() const { return simulated_time_; }
  void SetTimeStep(double dt) { time_step_ = dt; }
  double GetTimeStep() const { return time_step_; }

 private:
  std::string continuum_name_ = "";
  double time_step_ = std::numeric_limits<double>::max();
  double simulated_time_ = 0.0;
  double time_to_simulate_ = 0.0;
  int continuum_id_ = -1;

  BDM_CLASS_DEF(Continuum, 1);  // NOLINT
};

class ScalarField : public Continuum {
 public:
  ScalarField() = default;
  explicit ScalarField(const TRootIOCtor *) {}
  ~ScalarField() override = default;

  virtual double GetValue(const Double3 &position) const = 0;
  virtual Double3 GetGradient(const Double3 &position) const = 0;

  BDM_CLASS_DEF_OVERRIDE(ScalarField, 1);  // NOLINT
};

class VectorField : public Continuum {
 public:
  VectorField() = default;
  explicit VectorField(const TRootIOCtor *) {}
  ~VectorField() override = default;

  virtual Double3 GetValue(const Double3 &position) const = 0;
  virtual double GetDiv(const Double3 &position) const = 0;
  virtual double GetCurl(const Double3 &position) const = 0;

  BDM_CLASS_DEF_OVERRIDE(VectorField, 1);  // NOLINT
};

}  // namespace bdm

#endif  // CONTINUUM_INTERFACE_H_
