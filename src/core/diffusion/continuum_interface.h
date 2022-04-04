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
#include <string>
#include "core/container/math_array.h"

namespace bdm {

template <typename T>
class ContinuumModel {
 public:
  ContinuumModel() = default;
  explicit ContinuumModel(TRootIOCtor *p) {}
  virtual ~ContinuumModel() = default;

  virtual void Initialize() = 0;
  virtual void Update() = 0;
  virtual void Step(double dt) = 0;
  virtual T GetValue(const Double3 &position) const = 0;

  int GetContinuumId() const { return continuum_id_; }
  void SetContinuumId(int id) { continuum_id_ = id; }
  std::string GetContinuumName() const { return continuum_name_; }
  void SetContinuumName(const std::string name) { continuum_name_ = name; }

 private:
  int continuum_id_ = -1;
  std::string continuum_name_ = "";
};

class ScalarField : public ContinuumModel<double> {
 public:
  ScalarField() = default;
  explicit ScalarField(TRootIOCtor *p) {}
  virtual ~ScalarField() = default;

  virtual void Initialize() override = 0;
  virtual void Update() override = 0;
  virtual void Step(double dt) override = 0;
  virtual double GetValue(const Double3 &position) const override = 0;
  virtual Double3 GetGradient(const Double3 &position) const = 0;
};

class VectorField : public ContinuumModel<Double3> {
 public:
  VectorField() = default;
  explicit VectorField(TRootIOCtor *p) {}
  virtual ~VectorField() = default;

  virtual void Initialize() override = 0;
  virtual void Update() override = 0;
  virtual void Step(double dt) override = 0;
  virtual Double3 GetValue(const Double3 &position) const override = 0;
  virtual double GetDiv(const Double3 &position) const = 0;
  virtual double GetCurl(const Double3 &position) const = 0;
};

}  // namespace bdm

#endif  // CONTINUUM_INTERFACE_H_
