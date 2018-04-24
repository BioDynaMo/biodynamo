#ifndef DEFAULT_FORCE_H_
#define DEFAULT_FORCE_H_

#include <algorithm>
#include <cmath>

#include "backend.h"
#include "shape.h"
#include "math_util.h"
#include "log.h"
#include "random.h"

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
    Log::Fatal("DefaultForce", "DefaultForce only supports sphere or cylinder shapes");
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
    auto pP = cylinder->ProximalEnd();
    auto pD = cylinder->DistalEnd();
    auto axis = cylinder->GetSpringAxis();
    double actualLength =
        Math::Norm(axis);  // TODO use cylinder.GetActualLength() ??
    double d = cylinder->GetDiameter();
    auto c = sphere->GetPosition();
    double r = 0.5 * sphere->GetDiameter();

    // I. If the cylinder is small with respect to the sphere:
    // we only consider the interaction between the sphere and the point mass
    // (i.e. distal point) of the cylinder - that we treat as a sphere.
    if (actualLength < r) {
      //move back sphere center by 1 cylinder radius from pD
      // vector_x = rc * (axis[0]/actualLength)
      // vector_y = rc * (axis[1]/actualLength)
      // vector_z = rc * (axis[2]/actualLength)
      double rc = 0.5 * d;
      std::array<double, 3> dvec={rc*(axis[0]/actualLength), rc*(axis[1]/actualLength), rc*(axis[2]/actualLength)}; // displacement vector
      std::array<double, 3> npD = {pD[0]-dvec[0], pD[1]-dvec[1], pD[2]-dvec[2]}; // new sphere center
      *result = ComputeForceOfASphereOnASphere(npD, rc, c, r);
      return;
    }

    // II. If the cylinder is of the same scale or bigger than the sphere,
    // we look at the interaction between the sphere and the closest point
    // (to the sphere center) on the cylinder. This interaction is distributed
    // to
    // the two ends of the cylinder: the distal (point mass of the segment) and
    // the proximal (point mass of the mother of the segment).

    // 1)   Finding cc : the closest point to c on the line pPpD ("line" and not
    // "segment")
    //    It is the projection of the vector pP->c onto the vector pP->pD
    //    (=axis)
    auto pPc = Matrix::Subtract(c, pP);

    //    projection of pPc onto axis = (pPc.axis)/norm(axis)^2  * axis
    //    length of the projection = (pPc.axis)/norm(axis)

    double pPcDotAxis = pPc[0] * axis[0] + pPc[1] * axis[1] + pPc[2] * axis[2];
    double K = pPcDotAxis / (actualLength * actualLength);
    //    cc = pP + K* axis
    std::array<double, 3> cc{pP[0] + K * axis[0], pP[1] + K * axis[1],
                             pP[2] + K * axis[2]};

    // 2) Look if c -and hence cc- is (a) between pP and pD, (b) before pP or
    // (c) after pD
    double proportionTransmitedToProximalEnd;
    if (K <= 1.0 && K >= 0.0) {
      //    a)  if cc (the closest point to c on the line pPpD) is between pP
      //    and pD
      //      the force is distributed to the two nodes
      proportionTransmitedToProximalEnd = 1.0 - K;
    } else if (K < 0) {
      //    b)  if the closest point to c on the line pPpD is before pP
      //      the force is only on the proximal end (the mother point mass)
      proportionTransmitedToProximalEnd = 1.0;
      cc = pP;
    } else {  // if(K>1)
      //    c) if cc is after pD, the force is only on the distal end (the
      //    segment's point mass).
      proportionTransmitedToProximalEnd = 0.0;
      cc = pD;
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
    *result = {force[0], force[1], force[2], proportionTransmitedToProximalEnd};
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
    auto A = cylinder1->ProximalEnd();
    auto B = cylinder1->GetMassLocation();
    double d1 = cylinder1->GetDiameter();
    auto C = cylinder2->ProximalEnd();
    auto D = cylinder2->GetMassLocation();
    double d2 = cylinder2->GetDiameter();

    double K = 0.5;  // part devoted to the distal node

    //  looking for closest point on them
    // (based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/)
    double p13x = A[0] - C[0];
    double p13y = A[1] - C[1];
    double p13z = A[2] - C[2];
    double p43x = D[0] - C[0];
    double p43y = D[1] - C[1];
    double p43z = D[2] - C[2];
    double p21x = B[0] - A[0];
    double p21y = B[1] - A[1];
    double p21z = B[2] - A[2];

    double d1343 = p13x * p43x + p13y * p43y + p13z * p43z;
    double d4321 = p21x * p43x + p21y * p43y + p21z * p43z;
    double d1321 = p21x * p13x + p21y * p13y + p21z * p13z;
    double d4343 = p43x * p43x + p43y * p43y + p43z * p43z;
    double d2121 = p21x * p21x + p21y * p21y + p21z * p21z;

    std::array<double, 3> P1, P2;

    double denom = d2121 * d4343 - d4321 * d4321;

    // if the two segments are not ABSOLUTLY parallel
    if (denom > 0.000000000001) {  /// TODO hardcoded value
      double numer = d1343 * d4321 - d1321 * d4343;

      double mua = numer / denom;
      double mub = (d1343 + mua * d4321) / d4343;

      if (mua < 0) {
        P1 = A;
        K = 1;
      } else if (mua > 1) {
        P1 = B;
        K = 0;
      } else {
        P1 = std::array<double, 3>{A[0] + mua * p21x, A[1] + mua * p21y,
                                   A[2] + mua * p21z};
        K = 1 - mua;
      }

      if (mub < 0) {
        P2 = C;
      } else if (mub > 1) {
        P2 = D;
      } else {
        P2 = std::array<double, 3>{C[0] + mub * p43x, C[1] + mub * p43y,
                                   C[2] + mub * p43z};
      }

    } else {
      P1 = Matrix::Add(A, Matrix::ScalarMult(0.5, Matrix::Subtract(B, A)));
      P2 = Matrix::Add(C, Matrix::ScalarMult(0.5, Matrix::Subtract(D, C)));
    }

    // W put a virtual sphere on the two cylinders
    auto force = Matrix::ScalarMult(
        10, ComputeForceOfASphereOnASphere(P1, d1 / 2.0, P2, d2 / 2.0));

    *result = {force[0], force[1], force[2], K};
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
    if (distance_between_centers < 0.00000001) {  // TODO hard coded values
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
