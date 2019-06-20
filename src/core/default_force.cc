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

#include "core/default_force.h"

#include <algorithm>
#include <cmath>

#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "core/util/random.h"
#include "core/util/type.h"
#include "neuroscience/neurite_element.h"

namespace bdm {

using experimental::neuroscience::NeuriteElement;

Double4 DefaultForce::GetForce(const SimObject* lhs, const SimObject* rhs) {
  if (lhs->GetShape() == Shape::kSphere && rhs->GetShape() == Shape::kSphere) {
    Double3 result;
    ForceBetweenSpheres(lhs, rhs, &result);
    return {result[0], result[1], result[2], 0};
  } else if (lhs->GetShape() == Shape::kSphere &&
             rhs->GetShape() == Shape::kCylinder) {
    Double3 result;
    ForceOnASphereFromACylinder(lhs, rhs, &result);
    return {result[0], result[1], result[2], 0};
  } else if (lhs->GetShape() == Shape::kCylinder &&
             rhs->GetShape() == Shape::kSphere) {
    Double4 result;
    ForceOnACylinderFromASphere(lhs, rhs, &result);
    return result;
  } else if (lhs->GetShape() == Shape::kCylinder &&
             rhs->GetShape() == Shape::kCylinder) {
    Double4 result;
    ForceBetweenCylinders(lhs, rhs, &result);
    return result;
  } else {
    Log::Fatal("DefaultForce",
               "DefaultForce only supports sphere or cylinder shapes");
    return {0, 0, 0, 0};
  }
}

void DefaultForce::ForceBetweenSpheres(const SimObject* sphere_lhs,
                                       const SimObject* sphere_rhs,
                                       Double3* result) const {
  const Double3& ref_mass_location = sphere_lhs->GetPosition();
  double ref_diameter = sphere_lhs->GetDiameter();
  double ref_iof_coefficient = 0.15;
  const Double3& nb_mass_location = sphere_rhs->GetPosition();
  double nb_diameter = sphere_rhs->GetDiameter();
  double nb_iof_coefficient = 0.15;

  auto c1 = ref_mass_location;
  double r1 = 0.5 * ref_diameter;
  auto c2 = nb_mass_location;
  double r2 = 0.5 * nb_diameter;
  // We take virtual bigger radii to have a distant interaction, to get a
  // desired density.
  double additional_radius =
      10.0 * std::min(ref_iof_coefficient, nb_iof_coefficient);
  r1 += additional_radius;
  r2 += additional_radius;
  // the 3 components of the vector c2 -> c1
  double comp1 = c1[0] - c2[0];
  double comp2 = c1[1] - c2[1];
  double comp3 = c1[2] - c2[2];
  double center_distance =
      std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;
  // if no overlap : no force
  if (delta < 0) {
    *result = {0.0, 0.0, 0.0};
    return;
  }
  // to avoid a division by 0 if the centers are (almost) at the same
  //  location
  if (center_distance < 0.00000001) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto force2on1 = random->template UniformArray<3>(-3.0, 3.0);
    *result = force2on1;
    return;
  }
  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1;  // attraction coeff
  double k = 2;      // repulsion coeff
  double f = k * delta - gamma * std::sqrt(r * delta);

  double module = f / center_distance;
  Double3 force2on1({module * comp1, module * comp2, module * comp3});
  *result = force2on1;
}

void DefaultForce::ForceOnACylinderFromASphere(const SimObject* cylinder,
                                               const SimObject* sphere,
                                               Double4* result) const {
  auto* ne = bdm_static_cast<const NeuriteElement*>(cylinder);
  auto proximal_end = ne->ProximalEnd();
  auto distal_end = ne->DistalEnd();
  auto axis = ne->GetSpringAxis();
  // TODO(neurites) use cylinder.GetActualLength() ??
  double actual_length = axis.Norm();
  double d = ne->GetDiameter();
  auto c = sphere->GetPosition();
  double r = 0.5 * sphere->GetDiameter();

  // I. If the cylinder is small with respect to the sphere:
  // we only consider the interaction between the sphere and the point mass
  // (i.e. distal point) of the cylinder - that we treat as a sphere.
  if (actual_length < r) {
    // move back sphere center by 1 cylinder radius from distal_end
    // vector_x = rc * (axis[0]/actual_length)
    // vector_y = rc * (axis[1]/actual_length)
    // vector_z = rc * (axis[2]/actual_length)
    double rc = 0.5 * d;
    Double3 dvec = (axis / actual_length) * rc;  // displacement vector
    Double3 npd = distal_end - dvec;             // new sphere center
    *result = ComputeForceOfASphereOnASphere(npd, rc, c, r);
    return;
  }

  // II. If the cylinder is of the same scale or bigger than the sphere,
  // we look at the interaction between the sphere and the closest point
  // (to the sphere center) on the cylinder. This interaction is distributed
  // to
  // the two ends of the cylinder: the distal (point mass of the segment) and
  // the proximal (point mass of the mother of the segment).

  // 1)   Finding cc : the closest point to c on the line proximal_end
  // proximal_end
  // ("line" and not "segment")
  //    It is the projection of the vector proximal_end->c onto the vector
  //    proximal_end->distal_end
  //    (=axis)
  auto proximal_end_closest = c - proximal_end;

  //    projection of proximal_end_closest onto axis =
  //    (proximal_end_closest.axis)/norm(axis)^2  * axis
  //    length of the projection = (proximal_end_closest.axis)/norm(axis)
  double proximal_end_closest_axis =
      proximal_end_closest.EntryWiseProduct(axis).Sum();
  double k = proximal_end_closest_axis / (actual_length * actual_length);
  //    cc = proximal_end + k* axis
  Double3 cc = proximal_end + (axis * k);

  // 2) Look if c -and hence cc- is (a) between proximal_end and distal_end,
  // (b) before proximal_end or
  // (c) after distal_end
  double proportion_to_proximal_end;
  if (k <= 1.0 && k >= 0.0) {
    //    a)  if cc (the closest point to c on the line pPpD) is between
    //    proximal_end
    //    and distal_end
    //      the force is distributed to the two nodes
    proportion_to_proximal_end = 1.0 - k;
  } else if (k < 0) {
    //    b)  if the closest point to c on the line pPpD is before
    //    proximal_end
    //      the force is only on the proximal end (the mother point mass)
    proportion_to_proximal_end = 1.0;
    cc = proximal_end;
  } else {  // if(k>1)
    //    c) if cc is after distal_end, the force is only on the distal end
    //    (the
    //    segment's point mass).
    proportion_to_proximal_end = 0.0;
    cc = distal_end;
  }

  // 3)   If the smallest distance between the cylinder and the center of the
  // sphere
  //    is larger than the radius of the two objects , there is no
  //    interaction:
  double penetration = d / 2 + r - Math::GetL2Distance(c, cc);
  if (penetration <= 0) {
    *result = Double4{0.0, 0.0, 0.0, 0.0};
    return;
  }
  auto force = ComputeForceOfASphereOnASphere(cc, d * 0.5, c, r);
  *result = {force[0], force[1], force[2], proportion_to_proximal_end};
  return;
}

void DefaultForce::ForceOnASphereFromACylinder(const SimObject* sphere,
                                               const SimObject* cylinder,
                                               Double3* result) const {
  // it is the opposite of force on a cylinder from sphere:
  Double4 temp;
  ForceOnACylinderFromASphere(cylinder, sphere, &temp);

  *result = {-temp[0], -temp[1], -temp[2]};
}

void DefaultForce::ForceBetweenCylinders(const SimObject* cylinder1,
                                         const SimObject* cylinder2,
                                         Double4* result) const {
  auto* c1 = bdm_static_cast<const NeuriteElement*>(cylinder1);
  auto* c2 = bdm_static_cast<const NeuriteElement*>(cylinder2);
  auto a = c1->ProximalEnd();
  auto b = c1->GetMassLocation();
  double d1 = c1->GetDiameter();
  auto c = c2->ProximalEnd();
  auto d = c2->GetMassLocation();
  double d2 = c2->GetDiameter();

  double k = 0.5;  // part devoted to the distal node

  //  looking for closest point on them
  // (based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/)
  double p13x = a[0] - c[0];
  double p13y = a[1] - c[1];
  double p13z = a[2] - c[2];
  double p43x = d[0] - c[0];
  double p43y = d[1] - c[1];
  double p43z = d[2] - c[2];
  double p21x = b[0] - a[0];
  double p21y = b[1] - a[1];
  double p21z = b[2] - a[2];

  double d1343 = p13x * p43x + p13y * p43y + p13z * p43z;
  double d4321 = p21x * p43x + p21y * p43y + p21z * p43z;
  double d1321 = p21x * p13x + p21y * p13y + p21z * p13z;
  double d4343 = p43x * p43x + p43y * p43y + p43z * p43z;
  double d2121 = p21x * p21x + p21y * p21y + p21z * p21z;

  Double3 p1, p2;

  double denom = d2121 * d4343 - d4321 * d4321;

  // if the two segments are not ABSOLUTLY parallel
  if (denom > 0.000000000001) {  /// TODO(neurites) hardcoded value
    double numer = d1343 * d4321 - d1321 * d4343;

    double mua = numer / denom;
    double mub = (d1343 + mua * d4321) / d4343;

    if (mua < 0) {
      p1 = a;
      k = 1;
    } else if (mua > 1) {
      p1 = b;
      k = 0;
    } else {
      p1 = Double3{a[0] + mua * p21x, a[1] + mua * p21y, a[2] + mua * p21z};
      k = 1 - mua;
    }

    if (mub < 0) {
      p2 = c;
    } else if (mub > 1) {
      p2 = d;
    } else {
      p2 = Double3{c[0] + mub * p43x, c[1] + mub * p43y, c[2] + mub * p43z};
    }

  } else {
    p1 = a + (b - a) * 0.5;
    p2 = c + (d - c) * 0.5;
  }

  // W put a virtual sphere on the two cylinders
  auto force = ComputeForceOfASphereOnASphere(p1, d1 / 2.0, p2, d2 / 2.0) * 10;

  *result = {force[0], force[1], force[2], k};
}

Double4 DefaultForce::ComputeForceOfASphereOnASphere(const Double3& c1,
                                                     double r1,
                                                     const Double3& c2,
                                                     double r2) const {
  double comp1 = c1[0] - c2[0];
  double comp2 = c1[1] - c2[1];
  double comp3 = c1[2] - c2[2];
  double distance_between_centers =
      std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  double a = r1 + r2 - distance_between_centers;
  // if no overlap: no force
  if (a < 0) {
    return Double4{0.0, 0.0, 0.0, 0.0};
  }
  // to avoid a division by 0 if the centers are (almost) at the same location
  if (distance_between_centers <
      0.00000001) {  // TODO(neurites) hard coded values
    auto* random = Simulation::GetActive()->GetRandom();
    auto force2on1 = random->template UniformArray<3>(-3.0, 3.0);
    return Double4{force2on1[0], force2on1[1], force2on1[2], 0.0};
  } else {
    // the force is prop to the square of the interpentration distance and to
    // the radii.
    double module = a / distance_between_centers;
    Double4 force2on1({module * comp1, module * comp2, module * comp3, 0.0});
    return force2on1;
  }
}

}  // namespace bdm
