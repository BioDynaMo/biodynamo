#include "physics/default_force.h"

#include "matrix.h"

namespace bdm {
namespace physics {

DefaultForce::DefaultForce() {
}

DefaultForce::~DefaultForce() {
}

StringBuilder& DefaultForce::simStateToJson(StringBuilder& sb) const {
  sb.append("{}");
  return sb;
}

std::array<double, 3> DefaultForce::forceOnASphereFromASphere(PhysicalSphere* sphere1, PhysicalSphere* sphere2) const {
  auto c1 = sphere1->getMassLocation();
  double r1 = 0.5 * sphere1->getDiameter();
  auto c2 = sphere2->getMassLocation();
  double r2 = 0.5 * sphere2->getDiameter();
  // We take virtual bigger radii to have a distant interaction, to get a desired density.
  double additionalRadius = 10.0
      * std::min(sphere1->getInterObjectForceCoefficient(), sphere2->getInterObjectForceCoefficient());
  r1 += additionalRadius;
  r2 += additionalRadius;
  // the 3 components of the vector c2 -> c1
  double comp1 = c1[0] - c2[0];
  double comp2 = c1[1] - c2[1];
  double comp3 = c1[2] - c2[2];
  double distanceBetweenCenters = MathUtil::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - distanceBetweenCenters;
  // if no overlap : no force
  if (delta < 0)
    return std::array<double, 3>( { 0.0, 0.0, 0.0 });
  // to avoid a division by 0 if the centers are (almost) at the same location
  if (distanceBetweenCenters < 0.00000001) {
    auto force2on1 = Random::nextNoise(3.0);
    return force2on1;
  } else {
    // the force itself
    double R = (r1 * r2) / (r1 + r2);
    double gamma = 1;     // attraction coeff
    double k = 2;           // repulsion coeff
    double F = k * delta - gamma * MathUtil::sqrt(R * delta);

    double module = F / distanceBetweenCenters;
    std::array<double, 3> force2on1( { module * comp1, module * comp2, module * comp3 });
    return force2on1;
  }
}

std::array<double, 4> DefaultForce::forceOnACylinderFromASphere(PhysicalCylinder* cylinder,
                                                                PhysicalSphere* sphere) const {
  auto pP = cylinder->proximalEnd();
  auto pD = cylinder->distalEnd();
  auto axis = cylinder->getSpringAxis();
  double actualLength = Matrix::norm(axis);
  double d = cylinder->getDiameter();
  auto c = sphere->getMassLocation();
  double r = 0.5 * sphere->getDiameter();

  // I. If the cylinder is small with respect to the sphere:
  // we only consider the interaction between the sphere and the point mass
  // (i.e. distal point) of the cylinder - that we treat as a sphere.
  if (actualLength < r) {
    return computeForceOfASphereOnASphere(pD, d * 0.5, c, r);
  }

  // II. If the cylinder is of the same scale or bigger than the sphere,
  // we look at the interaction between the sphere and the closest point
  // (to the sphere center) on the cylinder. This interaction is distributed to
  // the two ends of the cylinder: the distal (point mass of the segment) and
  // the proximal (point mass of the mother of the segment).

  // 1)   Finding cc : the closest point to c on the line pPpD ("line" and not "segment")
  //    It is the projection of the vector pP->c onto the vector pP->pD (=axis)
  auto pPc = Matrix::subtract(c, pP);

  //    projection of pPc onto axis = (pPc.axis)/norm(axis)^2  * axis
  //    length of the projection = (pPc.axis)/norm(axis)

  double pPcDotAxis = pPc[0] * axis[0] + pPc[1] * axis[1] + pPc[2] * axis[2];
  double K = pPcDotAxis / (actualLength * actualLength);
  //    cc = pP + K* axis
  std::array<double, 3> cc { pP[0] + K * axis[0], pP[1] + K * axis[1], pP[2] + K * axis[2] };

  // 2) Look if c -and hence cc- is (a) between pP and pD, (b) before pP or (c) after pD
  double proportionTransmitedToProximalEnd;
  if (K <= 1.0 && K >= 0.0) {
    //    a)  if cc (the closest point to c on the line pPpD) is between pP and pD
    //      the force is distributed to the two nodes
    proportionTransmitedToProximalEnd = 1.0 - K;
  } else if (K < 0) {
    //    b)  if the closest point to c on the line pPpD is before pP
    //      the force is only on the proximal end (the mother point mass)
    proportionTransmitedToProximalEnd = 1.0;
    cc = pP;
  } else {     // if(K>1)
    //    c) if cc is after pD, the force is only on the distal end (the segment's point mass).
    proportionTransmitedToProximalEnd = 0.0;
    cc = pD;
  }

  // 3)   If the smallest distance between the cylinder and the center of the sphere
  //    is larger than the radius of the two objects , there is no interaction:
  double penetration = d / 2 + r - Matrix::distance(c, cc);
  if (penetration <= 0) {
    return std::array<double, 4> { 0.0, 0.0, 0.0, 0.0 };
  }
  auto force = computeForceOfASphereOnASphere(cc, d * 0.5, c, r);
  return std::array<double, 4> { force[0], force[1], force[2], proportionTransmitedToProximalEnd };
}

std::array<double, 3> DefaultForce::forceOnASphereFromACylinder(PhysicalSphere* sphere,
                                                                PhysicalCylinder* cylinder) const {
  // it is the opposite of force on a cylinder from sphere:
  auto temp = forceOnACylinderFromASphere(cylinder, sphere);
  return std::array<double, 3> { -temp[0], -temp[1], -temp[2] };
}

std::array<double, 4> DefaultForce::forceOnACylinderFromACylinder(PhysicalCylinder* cylinder1,
                                                                  PhysicalCylinder* cylinder2) const {
  auto A = cylinder1->proximalEnd();
  auto B = cylinder1->getMassLocation();
  double d1 = cylinder1->getDiameter();
  auto C = cylinder2->proximalEnd();
  auto D = cylinder2->getMassLocation();
  double d2 = cylinder2->getDiameter();

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

  //if the two segments are not ABSOLUTLY parallel
  if (denom > 0.000000000001) {
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
      P1 = std::array<double, 3> { A[0] + mua * p21x, A[1] + mua * p21y, A[2] + mua * p21z };
      K = 1 - mua;
    }

    if (mub < 0) {
      P2 = C;
    } else if (mub > 1) {
      P2 = D;
    } else {
      P2 = std::array<double, 3> { C[0] + mub * p43x, C[1] + mub * p43y, C[2] + mub * p43z };
    }

  } else {
    P1 = Matrix::add(A, Matrix::scalarMult(0.5, Matrix::subtract(B, A)));
    P2 = Matrix::add(C, Matrix::scalarMult(0.5, Matrix::subtract(D, C)));
  }

  // W put a virtual sphere on the two cylinders
  auto force = Matrix::scalarMult(10, computeForceOfASphereOnASphere(P1, d1 + 0, P2, d2 + 0));

  return std::array<double, 4> { force[0], force[1], force[2], K };
}

std::string DefaultForce::toString() const {
  return "DefaultForce@";  // + this;
}

std::array<double, 4> DefaultForce::computeForceOfASphereOnASphere(const std::array<double, 3>& c1, double r1,
                                                                   const std::array<double, 3>& c2, double r2) const {
  double comp1 = c1[0] - c2[0];
  double comp2 = c1[1] - c2[1];
  double comp3 = c1[2] - c2[2];
  double distanceBetweenCenters = MathUtil::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  double a = r1 + r2 - distanceBetweenCenters;
  // if no overlap : no force
  if (a < 0) {
    return std::array<double, 4> { 0.0, 0.0, 0.0, 0.0 };
  }
  // to avoid a division by 0 if the centers are (almost) at the same location
  if (distanceBetweenCenters < 0.00000001) {
    auto force2on1 = Random::nextNoise(3);
    return std::array<double, 4> { force2on1[0], force2on1[1], force2on1[2], 0.0 };
  } else {
    // the force is prop to the square of the interpentration distance and to the radii.
    double module = a / distanceBetweenCenters;
    std::array<double, 4> force2on1( { module * comp1, module * comp2, module * comp3, 0.0 });
    return force2on1;
  }
}

}  // namespace physics
}  // namespace bdm
