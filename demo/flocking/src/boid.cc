// -----------------------------------------------------------------------------
//
// Copyright (C) Moritz Grabmann.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "boid.h"
#include "sim_param.h"

namespace bdm {

// ---------------------------------------------------------------------------
// Double3 Methods
double NormSq(Double3 vector) {
  double res = 0;
  for (size_t i = 0; i < 3; i++)
    res += vector[i] * vector[i];
  return res;
};

Double3 UpperLimit(Double3 vector, double upper_limit) {
  double length = vector.Norm();
  if (length == 0) {
    return {0, 0, 0};
  }
  if (length > upper_limit) {
    vector = (vector / length) * upper_limit;
  }
  return vector;
};

Double3 GetNormalizedArray(Double3 vector) {
  if (vector.Norm() == 0)
    return {0, 0, 0};
  else
    return vector.GetNormalizedArray();
};

Double3 GetRandomVectorInUnitSphere() {
  auto* random = Simulation::GetActive()->GetRandom();

  double phi = random->Uniform(0, 2 * M_PI);
  double costheta = random->Uniform(-1, 1);
  double u = random->Uniform(0, 1);

  double theta = acos(costheta);
  double r = sqrt(u);

  double x_coord = r * sin(theta) * cos(phi);
  double y_coord = r * sin(theta) * sin(phi);
  double z_coord = r * cos(theta);

  Double3 vec = {x_coord, y_coord, z_coord};
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

Double3 Boid::CalculateDisplacement(const InteractionForce* force,
                                    double squared_radius, double dt) {
  Double3 zero = {0, 0, 0};
  return zero;
};

void Boid::ApplyDisplacement(const Double3& displacement) { ; };

const Double3& Boid::GetPosition() const { return position_; };

void Boid::SetPosition(const Double3& pos) { position_ = pos; };

double Boid::GetDiameter() const { return diameter_; };

void Boid::SetDiameter(double diameter) { diameter_ = diameter; };

// ---------------------------------------------------------------------------
// Important getter and setter

Double3 Boid::GetVelocity() const { return velocity_; };

void Boid::SetVelocity(Double3 velocity) {
  velocity_ = velocity;
  SetHeadingDirection(velocity_);
};

void Boid::SetBoidPerceptionRadius(double perception_radius) {
  boid_perception_radius_ = perception_radius;
  SetDiameter(boid_perception_radius_ * 2);
};

void Boid::SetPerceptionAngle(double angle) {
  cos_perception_angle_ = std::cos(angle);
};

void Boid::SetHeadingDirection(Double3 dir) {
  if (dir.Norm() != 0) {
    heading_direction_ = GetNormalizedArray(dir);
  }
};

double Boid::GetBoidInteractionRadius() { return boid_interaction_radius_; };

double Boid::GetBoidPerceptionRadius() { return boid_perception_radius_; };

// ---------------------------------------------------------------------------

bool Boid::CheckIfVisible(Double3 point) {
  if ((point - GetPosition()).Norm() == 0) {
    // identical points
    return true;
  }

  Double3 cone_normal = heading_direction_;
  Double3 direction_normal = (point - GetPosition()).GetNormalizedArray();
  double cos_angle = cone_normal * direction_normal;

  if (cos_angle >= cos_perception_angle_) {
    return true;
  } else {
    return false;
  }
};

Double3 Boid::SteerTowards(Double3 vector) {
  if (vector.Norm() == 0) {
    return {0, 0, 0};
  }
  Double3 steer = vector.GetNormalizedArray() * max_speed_ - velocity_;
  return steer;
  return UpperLimit(steer, max_acceleration_);
};

// ---------------------------------------------------------------------------
// Data Updates

void Boid::UpdateData() {
  // update velocity with current acceleration
  Double3 new_velocity_ = velocity_ + acceleration_ * d_t_;
  if (limit_speed_) {
    new_velocity_ = UpperLimit(new_velocity_, max_speed_);
  }
  SetVelocity(new_velocity_);

  // update position with new velocity
  Double3 new_position_ = GetPosition() + new_velocity_ * d_t_;
  SetPosition(new_position_);

  // reset acceleration acucumulator
  acceleration_ = {0, 0, 0};
  acceleration_accum_scalar = 0;
};

void Boid::AccelerationAccumulator(Double3 acc) {
  if (acceleration_accum_scalar + acc.Norm() <= max_acceleration_) {
    acceleration_accum_scalar += acc.Norm();
    acceleration_ += acc;
  } else {
    double s = max_acceleration_ - acceleration_accum_scalar;
    acceleration_accum_scalar = max_acceleration_;
    acceleration_ += acc * s;
  }
};

// ---------------------------------------------------------------------------
// Flocking Algorithm

Double3 Boid::GetFlockingForce() {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  CalculateNeighborData NeighborData(this);
  ctxt->ForEachNeighbor(NeighborData, *this, pow(boid_perception_radius_, 2));

  Double3 force = {0, 0, 0};
  force += NeighborData.GetU_a();
  force += GetExtendedCohesionTerm(NeighborData.GetCentreOfMass());

  return force;
};

Double3 Boid::GetNavigationalFeedbackForce() {
  return SteerTowards(pos_gamma_ - GetPosition()) * c_y_;
};

Double3 Boid::GetExtendedCohesionTerm(Double3 centre_of_mass) {
  double ratio =
      (centre_of_mass - GetPosition()).Norm() / boid_perception_radius_;
  double h_1 = boid_interaction_radius_ / boid_perception_radius_;
  double h_2 = h_1 * 1.2;

  double scale = Zeta(ratio, h_1, h_2);

  Double3 result =
      GetNormalizedArray(centre_of_mass - GetPosition()) * scale * c_a_3_;
  return result;
};

Double3 Boid::GetBoidInteractionTerm(const Boid* boid) {
  Double3 u_a = {0, 0, 0};

  // add gradient-based term to u_a
  Double3 n_ij = GetNormalizedArray(boid->GetPosition() - GetPosition());

  u_a += n_ij * Phi_a(Norm_sig(boid->GetPosition() - GetPosition())) * c_a_1_;

  // add consensus term
  double r_a = Norm_sig(boid_interaction_radius_);
  double a_ij =
      Rho_h(Norm_sig(boid->GetPosition() - GetPosition()) / r_a, h_a_);

  u_a += (boid->GetVelocity() - GetVelocity()) * a_ij * c_a_2_;

  return u_a;
};

double Boid::Norm_sig(Double3 z) {
  return (std::sqrt(1 + eps_ * NormSq(z)) - 1) / eps_;
};

double Boid::Norm_sig(double z) {
  return (std::sqrt(1 + eps_ * z * z) - 1) / eps_;
};

double Boid::Phi(double z) {
  // 0 < a <= b
  // "a" controls a max for attaction scaling, "b" min for repelling
  double a = 1;
  double b = 2.5;
  double c = std::abs(a - b) / std::sqrt(4 * a * b);
  return ((a + b) * Sigmoid(z + c) + (a - b)) / 2;
};

double Boid::Rho_h(double z, double h) {
  if (z >= 0 && z < h) {
    return 1;
  }
  if (z >= h && z <= 1) {
    return (1 + cos(M_PI * (z - h) / (1 - h))) / 2;
  }
  return 0;
};

double Boid::Rho_h_a(double z, double h) {
  if (z >= 0 && z < h) {
    return 1;
  }
  if (z >= h && z <= 1) {
    double scale = exp(-5 * (z - h) * (z - h));
    return scale * (1 + cos(M_PI * (z - h) / (1 - h))) / 2;
  }
  return 0;
};

double Boid::Zeta(double z, double h_1, double h_2) {
  if (z < h_1) {
    return 0;
  } else if (z >= h_1 && z <= h_2) {
    return (1 + cos(M_PI * (h_2 - z) / (h_2 - h_1))) / 2;
  } else {
    return 1;
  }
};

double Boid::Sigmoid(double z) { return z / (1 + std::abs(z)); };

double Boid::Phi_a(double z) {
  double r_a = Norm_sig(boid_interaction_radius_);
  double d_a = Norm_sig(neighbor_distance_);

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
                                       double squared_distance) {
  auto* neighbor_boid = bdm_static_cast<const Boid*>(neighbor);

  double dist = (boid_->GetPosition() - neighbor_boid->GetPosition()).Norm();
  bool is_visible = boid_->CheckIfVisible(neighbor_boid->GetPosition());

  if (is_visible && dist <= boid_->GetBoidInteractionRadius()) {
    u_a += boid_->GetBoidInteractionTerm(neighbor_boid);
  }

  if (is_visible && dist <= boid_->GetBoidPerceptionRadius()) {
    sum_pos += neighbor_boid->GetPosition();
    n++;
  }
};

Double3 CalculateNeighborData::GetCentreOfMass() {
  if (n != 0)
    return (sum_pos / n);
  else
    return boid_->GetPosition();
};

Double3 CalculateNeighborData::GetU_a() { return u_a; };

}  // namespace bdm
