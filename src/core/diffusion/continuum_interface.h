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

/// @brief Continuum class to interface with BioDynaMo for hybrid simulations.
///
/// This class is used to interface arbitrary Continuum implementations with
/// BioDynaMo. The continuum may be a single- or multi-threaded, or possibly
/// even a GPU implementation. Furthermore, it can be of arbitrary
/// dimension. We provide more specific interfaces for scalar and vector fields
/// (see `ScalarField` and `VectorField`). The interface is pure virtual, thus,
/// the user must implement the methods (`Initialize`, `Update`, `Step`) in the
/// derived class. The `DiffusionGrid` class is an example of such a derived
/// class implementing a continuum (scalar field) via a finite difference
/// discretization for the diffusion equation. Another example is given in the
/// demo `analytic_continuum` which attaches an analytic scalar field to the
/// simulation.
///
/// The class is included into the BioDynaMo simulation scheme through the
/// `ContinuumOp` class. This class is a stand-alone operation, i.e., it is
/// called once per simulation step (if its frequency is 1). This call to the
/// `ContinuumOp::operator()` method triggers the execution of the
/// `IntegrateTimeAsynchronously` method, which in turn calls the `Step` method
/// with appropriate time step(s). Note that there are two options to influence
/// the synchronisation of the simulation: the frequency of the `ContinuumOp`
/// and the time step of the `Continuum`. The former defines the synchronization
/// frequency between ABM and the continuum. The latter defines the time step of
/// the continuum.
class Continuum {
 public:
  Continuum() = default;
  explicit Continuum(const TRootIOCtor *) {}
  virtual ~Continuum() = default;

  /// Manages the time evolution of the continuum. The method is called by the
  /// `ContinuumOp::operator()` method. The method calles the `Step` method with
  /// the appropriate time step(s) as defined via the `time_step_` member. If
  /// the time step is not set, `dt` is used.
  void IntegrateTimeAsynchronously(real_t dt);

  /// Initializes the continuum. This method is called via
  /// `Scheduler::Initialize`. For some implementations, this method may be
  /// useful, other may not require it. A possibly use case is that agents move
  /// in a stationary continuum that is the solution to a timeindependent PDE.
  /// In this case, the `Initialize` method could be used to solve the PDE
  /// once at the beginning of the simulation.
  virtual void Initialize() = 0;

  /// Updates the continuum. Since this method is public, it can be called form
  /// anyone and trigger an update of the continuum model. We envision that this
  /// can be used for mesh refinements or for other purposes. The DiffusionGrid,
  /// for instance, reconstructs the grid under certain conditions with this
  /// member function.
  virtual void Update() = 0;

  /// This method integrates the continuum model in time by `dt`. Typically, the
  /// continuum is the solution of a partial differential equation (PDE). Thus,
  /// the step method is used to advance the solution in time. If your numerical
  /// scheme has requirements for it's parameters depending on `dt`, make sure
  /// to verify them in this method.
  virtual void Step(real_t dt) = 0;

  /// Returns the ID of the continuum.
  int GetContinuumId() const { return continuum_id_; }

  /// Sets the ID of the continuum.
  void SetContinuumId(int id) { continuum_id_ = id; }

  /// Returns the name of the continuum.
  const std::string &GetContinuumName() const { return continuum_name_; }

  /// Sets the name of the continuum.
  void SetContinuumName(const std::string &name) { continuum_name_ = name; }

  /// Returns the time simulated by the continuum.
  real_t GetSimulatedTime() const { return simulated_time_; }

  /// Sets the (max.) time step for the continuum time integration with `Step`.
  void SetTimeStep(real_t dt);

  /// Returns the time step for the continuum. If no time time step was set, it
  /// returns std::numeric_limits<real_t>::max(). Note however, the continuum
  /// uses the time step of the diffusion operation / simulation.
  real_t GetTimeStep() const;

 private:
  /// Name of the continuum.
  std::string continuum_name_ = "";

  /// Time step of the continuum (may differ from the simulation time step).
  real_t time_step_ = std::numeric_limits<real_t>::max();

  /// Passed simulation time for the continuum.
  real_t simulated_time_ = 0.0;

  /// Time that the continuum (still) has to integrate.
  real_t time_to_simulate_ = 0.0;

  /// Id of the continuum.
  int continuum_id_ = -1;

  BDM_CLASS_DEF(Continuum, 1);  // NOLINT
};

/// Interface for scalar fields. See `Continuum` for more information.
class ScalarField : public Continuum {
 public:
  ScalarField() = default;
  explicit ScalarField(const TRootIOCtor *) {}
  ~ScalarField() override = default;

  /// Returns the value of the scalar field at the given position.
  virtual real_t GetValue(const Real3 &position) const = 0;

  /// Returns the gradient of the scalar field at the given position.
  virtual Real3 GetGradient(const Real3 &position) const = 0;

  BDM_CLASS_DEF_OVERRIDE(ScalarField, 1);  // NOLINT
};

/// Interface for vector fields. See `Continuum` for more information.
class VectorField : public Continuum {
 public:
  VectorField() = default;
  explicit VectorField(const TRootIOCtor *) {}
  ~VectorField() override = default;

  /// Returns the value of the vector field at the given position.
  virtual Real3 GetValue(const Real3 &position) const = 0;

  /// Returns the divergence of the vector field at the given position.
  virtual real_t GetDiv(const Real3 &position) const = 0;

  /// Returns the curl of the vector field at the given position.
  virtual real_t GetCurl(const Real3 &position) const = 0;

  BDM_CLASS_DEF_OVERRIDE(VectorField, 1);  // NOLINT
};

}  // namespace bdm

#endif  // CONTINUUM_INTERFACE_H_
