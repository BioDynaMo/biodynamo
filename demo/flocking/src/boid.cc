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

#include "boid.h"
#include <cmath>
#include "sim_param.h"

namespace bdm {

// ---------------------------------------------------------------------------
// Real3 Methods
real_t NormSq(Real3 vector) {
  real_t res = 0;
  for (size_t i = 0; i < 3; i++)
    res += vector[i] * vector[i];
  return res;
};

Real3 UpperLimit(Real3 vector, real_t upper_limit) {
  real_t length = vector.Norm();
  if (length == 0) {
    return {0, 0, 0};
  }
  if (length > upper_limit) {
    vector = (vector / length) * upper_limit;
  }
  return vector;
};

Real3 GetNormalizedArray(Real3 vector) {
  if (vector.Norm() == 0)
    return {0, 0, 0};
  else
    return vector.GetNormalizedArray();
};

Real3 GetRandomVectorInUnitSphere() {
  auto* random = Simulation::GetActive()->GetRandom();

  real_t phi = random->Uniform(0, 2 * M_PI);
  real_t costheta = random->Uniform(-1, 1);
  real_t u = random->Uniform(0, 1);

  real_t theta = acos(costheta);
  real_t r = sqrt(u);

  real_t x_coord = r * sin(theta) * cos(phi);
  real_t y_coord = r * sin(theta) * sin(phi);
  real_t z_coord = r * cos(theta);

  Real3 vec = {x_coord, y_coord, z_coord};
  return vec;
};

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//
// Boid Class                                                                 //
//----------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////

void Boid::InitializeMembers() {
  const auto* param = Simulation::GetActive()->GetParam();
  const auto* sparam = param->Get<SimParam>();

  actual_diameter_ = sparam->actual_diameter;
  SetBoidPerceptionRadius(sparam->boid_perception_radius);
  boid_interaction_radius_ = sparam->boid_interaction_radius;
  SetPerceptionAngle((sparam->perception_angle_deg / 360) * M_PI);
  neighbor_distance_ = sparam->neighbor_distance;
  max_acceleration_ = sparam->max_accel;
  max_speed_ = sparam->max_speed;
  limit_speed_ = sparam->limit_speed;

  // Flocking constants
  c_a_1_ = sparam->c_a_1;
  c_a_2_ = sparam->c_a_2;
  c_a_3_ = sparam->c_a_3;
  c_y_ = sparam->c_y;
  h_a_ = sparam->h_a;
  eps_ = sparam->eps;
  d_t_ = sparam->d_t;
  pos_gamma_ = sparam->pos_gamma;
};

// ---------------------------------------------------------------------------
// Define necessary virtual functions of Base class

Shape Boid::GetShape() const { return Shape::kSphere; };

Real3 Boid::CalculateDisplacement(const InteractionForce* force,
                                  real_t squared_radius, real_t dt) {
  Real3 zero = {0, 0, 0};
  return zero;
};

void Boid::ApplyDisplacement(const Real3& displacement) { ; };

const Real3& Boid::GetPosition() const { return position_; };

void Boid::SetPosition(const Real3& pos) { position_ = pos; };

real_t Boid::GetDiameter() const { return diameter_; };

void Boid::SetDiameter(real_t diameter) { diameter_ = diameter; };

// ---------------------------------------------------------------------------
// Important getter and setter

Real3 Boid::GetVelocity() const { return velocity_; };

void Boid::SetVelocity(Real3 velocity) {
  velocity_ = velocity;
  SetHeadingDirection(velocity_);
};

void Boid::SetBoidPerceptionRadius(real_t perception_radius) {
  boid_perception_radius_ = perception_radius;
  SetDiameter(boid_perception_radius_ * 2);
};

void Boid::SetPerceptionAngle(real_t angle) {
  cos_perception_angle_ = std::cos(angle);
};

void Boid::SetHeadingDirection(Real3 dir) {
  if (dir.Norm() != 0) {
    heading_direction_ = GetNormalizedArray(dir);
  }
};

real_t Boid::GetBoidInteractionRadius() { return boid_interaction_radius_; };

real_t Boid::GetBoidPerceptionRadius() { return boid_perception_radius_; };

// ---------------------------------------------------------------------------

bool Boid::CheckIfVisible(Real3 point) {
  if ((point - GetPosition()).Norm() == 0) {
    // identical points
    return true;
  }

  Real3 cone_normal = heading_direction_;
  Real3 direction_normal = (point - GetPosition()).GetNormalizedArray();
  real_t cos_angle = cone_normal * direction_normal;

  if (cos_angle >= cos_perception_angle_) {
    return true;
  } else {
    return false;
  }
};

Real3 Boid::SteerTowards(Real3 vector) {
  if (vector.Norm() == 0) {
    return {0, 0, 0};
  }
  Real3 steer = vector.GetNormalizedArray() * max_speed_ - velocity_;
  return steer;
  return UpperLimit(steer, max_acceleration_);
};

// ---------------------------------------------------------------------------
// Data Updates

void Boid::UpdateData() {
  // update velocity with current acceleration
  Real3 new_velocity_ = velocity_ + acceleration_ * d_t_;
  if (limit_speed_) {
    new_velocity_ = UpperLimit(new_velocity_, max_speed_);
  }
  SetVelocity(new_velocity_);

  // update position with new velocity
  Real3 new_position_ = GetPosition() + new_velocity_ * d_t_;
  SetPosition(new_position_);

  // reset acceleration acucumulator
  acceleration_ = {0, 0, 0};
  acceleration_accum_scalar = 0;
};

void Boid::AccelerationAccumulator(Real3 acc) {
  if (acceleration_accum_scalar + acc.Norm() <= max_acceleration_) {
    acceleration_accum_scalar += acc.Norm();
    acceleration_ += acc;
  } else {
    real_t s = max_acceleration_ - acceleration_accum_scalar;
    acceleration_accum_scalar = max_acceleration_;
    acceleration_ += acc * s;
  }
};

// ---------------------------------------------------------------------------
// Flocking Algorithm

Real3 Boid::GetFlockingForce() {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  CalculateNeighborData NeighborData(this);
  ctxt->ForEachNeighbor(NeighborData, *this, pow(boid_perception_radius_, 2));

  Real3 force = {0, 0, 0};
  force += NeighborData.GetU_a();
  force += GetExtendedCohesionTerm(NeighborData.GetCentreOfMass());

  return force;
};

Real3 Boid::GetNavigationalFeedbackForce() {
  return SteerTowards(pos_gamma_ - GetPosition()) * c_y_;
};

Real3 Boid::GetExtendedCohesionTerm(Real3 centre_of_mass) {
  real_t ratio =
      (centre_of_mass - GetPosition()).Norm() / boid_perception_radius_;
  real_t h_1 = boid_interaction_radius_ / boid_perception_radius_;
  real_t h_2 = h_1 * 1.2;

  real_t scale = Zeta(ratio, h_1, h_2);

  Real3 result =
      GetNormalizedArray(centre_of_mass - GetPosition()) * scale * c_a_3_;
  return result;
};

Real3 Boid::GetBoidInteractionTerm(const Boid* boid) {
  Real3 u_a = {0, 0, 0};

  // add gradient-based term to u_a
  Real3 n_ij = GetNormalizedArray(boid->GetPosition() - GetPosition());

  u_a += n_ij * Phi_a(Norm_sig(boid->GetPosition() - GetPosition())) * c_a_1_;

  // add consensus term
  real_t r_a = Norm_sig(boid_interaction_radius_);
  real_t a_ij =
      Rho_h(Norm_sig(boid->GetPosition() - GetPosition()) / r_a, h_a_);

  u_a += (boid->GetVelocity() - GetVelocity()) * a_ij * c_a_2_;

  return u_a;
};

real_t Boid::Norm_sig(Real3 z) {
  return (std::sqrt(1 + eps_ * NormSq(z)) - 1) / eps_;
};

real_t Boid::Norm_sig(real_t z) {
  return (std::sqrt(1 + eps_ * z * z) - 1) / eps_;
};

real_t Boid::Phi(real_t z) {
  // 0 < a <= b
  // "a" controls a max for attaction scaling, "b" min for repelling
  real_t a = 1;
  real_t b = 2.5;
  real_t c = std::abs(a - b) / std::sqrt(4 * a * b);
  return ((a + b) * Sigmoid(z + c) + (a - b)) / 2;
};

real_t Boid::Rho_h(real_t z, real_t h) {
  if (z >= 0 && z < h) {
    return 1;
  }
  if (z >= h && z <= 1) {
    return (1 + cos(M_PI * (z - h) / (1 - h))) / 2;
  }
  return 0;
};

real_t Boid::Rho_h_a(real_t z, real_t h) {
  if (z >= 0 && z < h) {
    return 1;
  }
  if (z >= h && z <= 1) {
    real_t scale = exp(-5 * (z - h) * (z - h));
    return scale * (1 + cos(M_PI * (z - h) / (1 - h))) / 2;
  }
  return 0;
};

real_t Boid::Zeta(real_t z, real_t h_1, real_t h_2) {
  if (z < h_1) {
    return 0;
  } else if (z >= h_1 && z <= h_2) {
    return (1 + cos(M_PI * (h_2 - z) / (h_2 - h_1))) / 2;
  } else {
    return 1;
  }
};

real_t Boid::Sigmoid(real_t z) { return z / (1 + std::abs(z)); };

real_t Boid::Phi_a(real_t z) {
  real_t r_a = Norm_sig(boid_interaction_radius_);
  real_t d_a = Norm_sig(neighbor_distance_);

  return Rho_h_a(z / r_a, h_a_) * Phi(z - d_a);
};

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------//
// Flocking Behaviour                                                         //
//----------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////

void Flocking::Run(Agent* agent) {
  auto* boid = dynamic_cast<Boid*>(agent);

  boid->AccelerationAccumulator(boid->GetFlockingForce());
  boid->AccelerationAccumulator(boid->GetNavigationalFeedbackForce());
};

void CalculateNeighborData::operator()(Agent* neighbor,
                                       real_t squared_distance) {
  auto* neighbor_boid = bdm_static_cast<const Boid*>(neighbor);

  real_t distance = std::sqrt(squared_distance);
  bool is_visible = boid_->CheckIfVisible(neighbor_boid->GetPosition());

  if (is_visible && distance <= boid_->GetBoidInteractionRadius()) {
    u_a += boid_->GetBoidInteractionTerm(neighbor_boid);
  }

  if (is_visible && distance <= boid_->GetBoidPerceptionRadius()) {
    sum_pos += neighbor_boid->GetPosition();
    n++;
  }
};

Real3 CalculateNeighborData::GetCentreOfMass() {
  if (n != 0)
    return (sum_pos / n);
  else
    return boid_->GetPosition();
};

Real3 CalculateNeighborData::GetU_a() { return u_a; };

}  // namespace bdm
