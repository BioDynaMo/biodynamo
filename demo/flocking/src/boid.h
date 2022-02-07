// -----------------------------------------------------------------------------
//
// Copyright (C) Moritz Grabmann.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef BOID_H_
#define BOID_H_

#include "core/behavior/behavior.h"
#include "core/container/math_array.h"
#include "core/functor.h"

namespace bdm {

// ---------------------------------------------------------------------------
// Double3 Methods

double NormSq(Double3 vector);

Double3 UpperLimit(Double3 vector, double upper_limit);

Double3 GetNormalizedArray(Double3 vector);

Double3 GetRandomVectorInUnitSphere();

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//
// Boid Class                                                                 //
//----------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////

class Boid : public Agent {
  BDM_AGENT_HEADER(Boid, Agent, 1);

 public:
  Boid() {}
  explicit Boid(const Double3& position)
      : position_(position), diameter_(1.0) {}
  virtual ~Boid() {}

  // Initializes Boid parameters with given SimParam
  void InitializeMembers();

  // ---------------------------------------------------------------------------
  // Define necessary virtual functions of Base class. Those functions are
  // called from BioDynaMo's main engine but we don't need that here. Thus,
  // the function return zero or are defined as an empty call.

  Shape GetShape() const override;

  Double3 CalculateDisplacement(const InteractionForce* force,
                                double squared_radius, double dt) override;

  void ApplyDisplacement(const Double3& displacement) override;

  const Double3& GetPosition() const override;

  void SetPosition(const Double3& pos) override;

  double GetDiameter() const override;

  void SetDiameter(double diameter) override;

  // ---------------------------------------------------------------------------
  // Important getter and setter

  Double3 GetVelocity() const;

  void SetVelocity(Double3 velocity);

  void SetBoidPerceptionRadius(double perception_radius);

  void SetPerceptionAngle(double angle);

  void SetHeadingDirection(Double3 dir);

  double GetBoidInteractionRadius();

  double GetBoidPerceptionRadius();

  // ---------------------------------------------------------------------------
  // Checks if the point is inside the viewing cone defined by
  // heading_direction_ and perception_angle_
  bool CheckIfVisible(Double3 point);

  // Returns a Steering-Force towards the argument vector
  Double3 SteerTowards(Double3 vector);

  // ---------------------------------------------------------------------------
  // Data Updates

  // Update position and velocity after a computational step
  void UpdateData();

  // Accumulates acceleration terms in a priority based scheme
  void AccelerationAccumulator(Double3 acceleration_to_add);

  // ---------------------------------------------------------------------------
  // Flocking Algorithm

  // iterates over all neighbor boids and adds the interaction terms;
  // returns a flocking force that produces an a-latice structure
  Double3 GetFlockingForce();

  // Returns an acceleration term towards pos_gamma_
  Double3 GetNavigationalFeedbackForce();

  // Returns an acceleration term that improves the overall flock cohesion
  Double3 GetExtendedCohesionTerm(Double3 centre_of_mass);

  // Returns the interaction term for a given boid
  Double3 GetBoidInteractionTerm(const Boid* boid);

  // Functions needed to calculate the interaction terms
  double Norm_sig(Double3 z);

  double Norm_sig(double z);

  double Phi(double z);

  double Rho_h(double z, double h);

  double Rho_h_a(double z, double h);

  double Zeta(double z, double h_onset, double h_maxeff);

  double Sigmoid(double z);

  double Phi_a(double z);

  // ---------------------------------------------------------------------------
 private:
  Double3 position_, velocity_, heading_direction_, acceleration_;
  double acceleration_accum_scalar;
  double diameter_, actual_diameter_;
  double boid_perception_radius_, boid_interaction_radius_;
  double cos_perception_angle_;
  double neighbor_distance_;
  double max_acceleration_, max_speed_;
  bool limit_speed_;

  // Flocking constants
  double c_a_1_, c_a_2_, c_a_3_, c_y_;
  double h_a_, eps_, d_t_;
  Double3 pos_gamma_;  // gamma agent location (common group objective)
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
class CalculateNeighborData : public Functor<void, Agent*, double> {
 public:
  CalculateNeighborData(Boid* boid) : boid_(boid) {}
  virtual ~CalculateNeighborData() {}

  void operator()(Agent* neighbor, double squared_distance) override;

  Double3 GetU_a();

  Double3 GetCentreOfMass();

  Boid* boid_;
  Double3 u_a = {0, 0, 0}, sum_pos = {0, 0, 0};
  int n = 0;
};

}  // namespace bdm

#endif  // BOID_H_
