#ifndef DEFAULT_FORCE_H_
#define DEFAULT_FORCE_H_

#include <algorithm>
#include <cmath>

#include "backend.h"
#include "log.h"
#include "math_util.h"
#include "random.h"
#include "shape.h"

namespace bdm {

class DefaultForce {
 public:
  DefaultForce() {}
  ~DefaultForce() {}
  DefaultForce(const DefaultForce&) = delete;
  DefaultForce& operator=(const DefaultForce&) = delete;

  template <typename TLhs, typename TRhs>
  typename std::enable_if<TLhs::GetShape() == kSphere &&
                              TRhs::GetShape() == kSphere,
                          std::array<double, 3>>::type
  GetForce(const TLhs* lhs, const TRhs* rhs) const {
    std::array<double, 3> result;
    ForceBetweenSpheres(lhs, rhs, &result);
    return result;
  }

  template <typename TLhs, typename TRhs>
  typename std::enable_if<TLhs::GetShape() == kSphere &&
                              TRhs::GetShape() == kCylinder,
                          std::array<double, 3>>::type
  GetForce(const TLhs* lhs, const TRhs* rhs) const {
    std::array<double, 3> result;
    ForceOnASphereFromACylinder(lhs, rhs, &result);
    return result;
  }

  template <typename TLhs, typename TRhs>
  typename std::enable_if<TLhs::GetShape() == kCylinder &&
                              TRhs::GetShape() == kSphere,
                          std::array<double, 4>>::type
  GetForce(const TLhs* lhs, const TRhs* rhs) const {
    std::array<double, 4> result;
    ForceOnACylinderFromASphere(lhs, rhs, &result);
    return result;
  }

  template <typename TLhs, typename TRhs>
  typename std::enable_if<TLhs::GetShape() == kCylinder &&
                              TRhs::GetShape() == kCylinder,
                          std::array<double, 4>>::type
  GetForce(const TLhs* lhs, const TRhs* rhs) const {
    std::array<double, 4> result;
    ForceBetweenCylinders(lhs, rhs, &result);
    return result;
  }

  std::array<double, 3> GetForce(...) const {
    Log::Fatal("DefaultForce",
               "DefaultForce only supports sphere or cylinder shapes");
    return {0, 0, 0};
  }

 private:
  template <typename TSphereLhs, typename TSphereRhs>
  void ForceBetweenSpheres(const TSphereLhs* sphere_lhs,
                           const TSphereRhs* sphere_rhs,
                           std::array<double, 3>* result) const {
    const std::array<double, 3>& ref_mass_location = sphere_lhs->GetPosition();
    double ref_diameter = sphere_lhs->GetDiameter();
    double ref_iof_coefficient = 0.15;
    const std::array<double, 3>& nb_mass_location = sphere_rhs->GetPosition();
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
      auto force2on1 = gRandom.NextNoise(3.0);
      *result = force2on1;
      return;
    }
    // the force itself
    double r = (r1 * r2) / (r1 + r2);
    double gamma = 1;  // attraction coeff
    double k = 2;      // repulsion coeff
    double f = k * delta - gamma * std::sqrt(r * delta);

    double module = f / center_distance;
    std::array<double, 3> force2on1(
        {module * comp1, module * comp2, module * comp3});
    *result = force2on1;
  }

  template <typename TCylinder, typename TSphere>
  void ForceOnACylinderFromASphere(const TCylinder* cylinder,
                                   const TSphere* sphere,
                                   std::array<double, 4>* result) const {
    auto proximal_end = cylinder->ProximalEnd();
    auto distal_end = cylinder->DistalEnd();
    auto axis = cylinder->GetSpringAxis();
    // TODO(neurites) use cylinder.GetActualLength() ??
    double actual_length = Math::Norm(axis);
    double d = cylinder->GetDiameter();
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
      std::array<double, 3> dvec = {
          rc * (axis[0] / actual_length), rc * (axis[1] / actual_length),
          rc * (axis[2] / actual_length)};  // displacement vector
      std::array<double, 3> npd = {
          distal_end[0] - dvec[0], distal_end[1] - dvec[1],
          distal_end[2] - dvec[2]};  // new sphere center
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
    auto proximal_end_closest = Math::Subtract(c, proximal_end);

    //    projection of proximal_end_closest onto axis =
    //    (proximal_end_closest.axis)/norm(axis)^2  * axis
    //    length of the projection = (proximal_end_closest.axis)/norm(axis)

    double proximal_end_closest_axis = proximal_end_closest[0] * axis[0] +
                                       proximal_end_closest[1] * axis[1] +
                                       proximal_end_closest[2] * axis[2];
    double k = proximal_end_closest_axis / (actual_length * actual_length);
    //    cc = proximal_end + k* axis
    std::array<double, 3> cc{proximal_end[0] + k * axis[0],
                             proximal_end[1] + k * axis[1],
                             proximal_end[2] + k * axis[2]};

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
      *result = std::array<double, 4>{0.0, 0.0, 0.0, 0.0};
      return;
    }
    auto force = ComputeForceOfASphereOnASphere(cc, d * 0.5, c, r);
    *result = {force[0], force[1], force[2], proportion_to_proximal_end};
    return;
  }

  template <typename TCylinder, typename TSphere>
  void ForceOnASphereFromACylinder(const TSphere* sphere,
                                   const TCylinder* cylinder,
                                   std::array<double, 3>* result) const {
    // it is the opposite of force on a cylinder from sphere:
    std::array<double, 4> temp;
    ForceOnACylinderFromASphere(cylinder, sphere, &temp);

    *result = {-temp[0], -temp[1], -temp[2]};
  }

  template <typename TCylinderLhs, typename TCylinderRhs>
  void ForceBetweenCylinders(const TCylinderLhs* cylinder1,
                             const TCylinderRhs* cylinder2,
                             std::array<double, 4>* result) const {
    auto a = cylinder1->ProximalEnd();
    auto b = cylinder1->GetMassLocation();
    double d1 = cylinder1->GetDiameter();
    auto c = cylinder2->ProximalEnd();
    auto d = cylinder2->GetMassLocation();
    double d2 = cylinder2->GetDiameter();

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

    std::array<double, 3> p1, p2;

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
        p1 = std::array<double, 3>{a[0] + mua * p21x, a[1] + mua * p21y,
                                   a[2] + mua * p21z};
        k = 1 - mua;
      }

      if (mub < 0) {
        p2 = c;
      } else if (mub > 1) {
        p2 = d;
      } else {
        p2 = std::array<double, 3>{c[0] + mub * p43x, c[1] + mub * p43y,
                                   c[2] + mub * p43z};
      }

    } else {
      p1 = Math::Add(a, Math::ScalarMult(0.5, Math::Subtract(b, a)));
      p2 = Math::Add(c, Math::ScalarMult(0.5, Math::Subtract(d, c)));
    }

    // W put a virtual sphere on the two cylinders
    auto force = Math::ScalarMult(
        10, ComputeForceOfASphereOnASphere(p1, d1 / 2.0, p2, d2 / 2.0));

    *result = {force[0], force[1], force[2], k};
  }

  std::array<double, 4> ComputeForceOfASphereOnASphere(
      const std::array<double, 3>& c1, double r1,
      const std::array<double, 3>& c2, double r2) const {
    double comp1 = c1[0] - c2[0];
    double comp2 = c1[1] - c2[1];
    double comp3 = c1[2] - c2[2];
    double distance_between_centers =
        std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
    // the overlap distance (how much one penetrates in the other)
    double a = r1 + r2 - distance_between_centers;
    // if no overlap: no force
    if (a < 0) {
      return std::array<double, 4>{0.0, 0.0, 0.0, 0.0};
    }
    // to avoid a division by 0 if the centers are (almost) at the same location
    if (distance_between_centers <
        0.00000001) {  // TODO(neurites) hard coded values
      auto force2on1 = gRandom.NextNoise(3.0);
      return std::array<double, 4>{force2on1[0], force2on1[1], force2on1[2],
                                   0.0};
    } else {
      // the force is prop to the square of the interpentration distance and to
      // the radii.
      double module = a / distance_between_centers;
      std::array<double, 4> force2on1(
          {module * comp1, module * comp2, module * comp3, 0.0});
      return force2on1;
    }
  }
};

}  // namespace bdm

#endif  // DEFAULT_FORCE_H_
