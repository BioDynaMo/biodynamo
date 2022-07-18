// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// Author: Moritz Grabmann (2022)
//
// -----------------------------------------------------------------------------

#ifndef BOID_H_
#define BOID_H_

#include "core/behavior/behavior.h"
#include "core/container/math_array.h"
#include "core/functor.h"

namespace bdm {

// ---------------------------------------------------------------------------
// Real3 Methods

real_t NormSq(Real3 vector);

Real3 UpperLimit(Real3 vector, real_t upper_limit);

Real3 GetNormalizedArray(Real3 vector);

Real3 GetRandomVectorInUnitSphere();

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//
// Boid Class                                                                 //
//----------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////

class Boid : public Agent {
  BDM_AGENT_HEADER(Boid, Agent, 1);

 public:
  Boid() {}
  explicit Boid(const Real3& position) : position_(position), diameter_(1.0) {}
  virtual ~Boid() {}

  // Initializes Boid parameters with given SimParam
  void InitializeMembers();

  // ---------------------------------------------------------------------------
  // Define necessary virtual functions of Base class. Those functions are
  // called from BioDynaMo's main engine but we don't need that here. Thus,
  // the function return zero or are defined as an empty call.

  Shape GetShape() const override;

  Real3 CalculateDisplacement(const InteractionForce* force,
                              real_t squared_radius, real_t dt) override;

  void ApplyDisplacement(const Real3& displacement) override;

  const Real3& GetPosition() const override;

  void SetPosition(const Real3& pos) override;

  real_t GetDiameter() const override;

  void SetDiameter(real_t diameter) override;

  // ---------------------------------------------------------------------------
  // Important getter and setter

  Real3 GetVelocity() const;

  void SetVelocity(Real3 velocity);

  void SetBoidPerceptionRadius(real_t perception_radius);

  void SetPerceptionAngle(real_t angle);

  void SetHeadingDirection(Real3 dir);

  real_t GetBoidInteractionRadius();

  real_t GetBoidPerceptionRadius();

  // ---------------------------------------------------------------------------
  // Checks if the point is inside the viewing cone defined by
  // heading_direction_ and perception_angle_
  bool CheckIfVisible(Real3 point);

  // Returns a Steering-Force towards the argument vector
  Real3 SteerTowards(Real3 vector);

  // ---------------------------------------------------------------------------
  // Data Updates

  // Update position and velocity after a computational step
  void UpdateData();

  // Accumulates acceleration terms in a priority based scheme
  void AccelerationAccumulator(Real3 acceleration_to_add);

  // ---------------------------------------------------------------------------
  // Flocking Algorithm

  // iterates over all neighbor boids and adds the interaction terms;
  // returns a flocking force that produces an a-latice structure
  Real3 GetFlockingForce();

  // Returns an acceleration term towards pos_gamma_
  Real3 GetNavigationalFeedbackForce();

  // Returns an acceleration term that improves the overall flock cohesion
  Real3 GetExtendedCohesionTerm(Real3 centre_of_mass);

  // Returns the interaction term for a given boid
  Real3 GetBoidInteractionTerm(const Boid* boid);

  // Functions needed to calculate the interaction terms
  real_t Norm_sig(Real3 z);

  real_t Norm_sig(real_t z);

  real_t Phi(real_t z);

  real_t Rho_h(real_t z, real_t h);

  real_t Rho_h_a(real_t z, real_t h);

  real_t Zeta(real_t z, real_t h_onset, real_t h_maxeff);

  real_t Sigmoid(real_t z);

  real_t Phi_a(real_t z);

  // ---------------------------------------------------------------------------
 private:
  Real3 position_, velocity_, heading_direction_, acceleration_;
  real_t acceleration_accum_scalar;
  real_t diameter_, actual_diameter_;
  real_t boid_perception_radius_, boid_interaction_radius_;
  real_t cos_perception_angle_;
  real_t neighbor_distance_;
  real_t max_acceleration_, max_speed_;
  bool limit_speed_;

  // Flocking constants
  real_t c_a_1_, c_a_2_, c_a_3_, c_y_;
  real_t h_a_, eps_, d_t_;
  Real3 pos_gamma_;  // gamma agent location (common group objective)
};

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//
// Flocking Behaviour                                                         //
//----------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////

struct Flocking : public Behavior {
  BDM_BEHAVIOR_HEADER(Flocking, Behavior, 1);

  void Run(Agent* agent) override;
};

// Functor class needed to calculate neighbor data in Flocking
// ForEachNeighbor call
class CalculateNeighborData : public Functor<void, Agent*, real_t> {
 public:
  CalculateNeighborData(Boid* boid) : boid_(boid) {}
  virtual ~CalculateNeighborData() {}

  void operator()(Agent* neighbor, real_t squared_distance) override;

  Real3 GetU_a();

  Real3 GetCentreOfMass();

  Boid* boid_;
  Real3 u_a = {0, 0, 0}, sum_pos = {0, 0, 0};
  int n = 0;
};

}  // namespace bdm

#endif  // BOID_H_
