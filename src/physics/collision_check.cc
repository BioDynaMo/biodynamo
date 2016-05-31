#include "physics/collision_check.h"

#include "matrix.h"
#include "stl_util.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"

namespace cx3d {
namespace physics {

double CollisionCheck::howMuchCanWeMove(const std::array<double, 3>& A, const std::array<double, 3>& B,
                                        const std::array<double, 3>& C, const std::array<double, 3>& D,
                                        const std::array<double, 3>& E, double dim) {
  std::array<double, 3> vec_1, vec_2, vec_3, vec_4, n, vec_normal_a, vec_normal_b, vec_normal_c;
  vec_1 = Matrix::subtract(B, A);
  vec_2 = Matrix::subtract(C, A);
  vec_3 = Matrix::subtract(C, B);
  n = Matrix::crossProduct(vec_1, vec_2);
  // plane through A and perpendicular to vecNormal (= plane through A,B and C):
  // (good summary on planes at http://mathworld.wolfram.com/Plane.html ).
  // Basically, the  plane perpendicular to n = (a,b,c) and containing the point Xo = (xo,yo,zo)
  // contains the points X = (x,y,z) satisfying : n dot (X-Xo) = 0
  // This last equation gives a*x + b*y + c*z = d, with d = a*xo + b*yo + c*zo
  // Here we take A as our point Xo.
  double a, b, c, d;
  a = n[0];
  b = n[1];
  c = n[2];
  d = Matrix::dot(A, n);
  // H = intersection point of previous plane and line DE
  // line = E + lambda(D-E). We look for a lambda s.t for each component
  // the equation of the previous plane is satified
  double lambda = (d - (a * E[0] + b * E[1] + c * E[2])) / (a * (D[0] - E[0]) + b * (D[1] - E[1]) + c * (D[2] - E[2]));
  // Is the intersection point between D and E ?
  if (lambda > 1 || lambda < 0) {
    return 1.0;
  }
  auto H = Matrix::add(E, Matrix::scalarMult(lambda, Matrix::subtract(D, E)));
  // Is H inside the triangle ABC? yes if it satisfies three conditions:
  // i) H and C are on the same side of vec1
  auto side_1 = Matrix::subtract(H, A);
  auto side_2 = Matrix::subtract(C, A);
  vec_normal_a = Matrix::normalize(Matrix::crossProduct(n, vec_1));
  if (STLUtil::sgn(Matrix::dot(vec_normal_a, side_1) * Matrix::dot(vec_normal_a, side_2)) < 0) {
    return 1.0;
  }
  // ii) H and B are on the same side of vec_2
  side_1 = Matrix::subtract(H, A);
  side_2 = Matrix::subtract(B, A);
  vec_normal_b = Matrix::crossProduct(n, vec_2);
  if (STLUtil::sgn(Matrix::dot(vec_normal_b, side_1) * Matrix::dot(vec_normal_b, side_2)) < 0) {
    return 1.0;
  }
  // iii) H and A are on the same side of vec_3
  side_1 = Matrix::subtract(H, B);
  side_2 = Matrix::subtract(A, B);
  vec_normal_c = Matrix::crossProduct(n, vec_3);
  if (STLUtil::sgn(Matrix::dot(vec_normal_c, side_1) * Matrix::dot(vec_normal_c, side_2)) < 0) {
    return 1.0;
  }

  // If we arrive here it means that the cylinder D-E is inside the area swaped by the moving cylinder.
  // H is the point where D-E crosses the triangle ABC.
  // Now we look where on the line AB we can go so that the smaller triangle doesn't enters D-E
  // We use the 2 line segments algorithm described in http://mathworld.wolfram.com/Line-LineIntersection.html

  vec_4 = Matrix::subtract(H, A);
  vec_1 = std::array<double, 3> { -vec_1[0], -vec_1[1], -vec_1[2] };
  auto temp = Matrix::crossProduct(vec_3, vec_4);

  double s = Matrix::dot(Matrix::crossProduct(vec_1, vec_4), temp);
  double s_numerator = temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2];
  s /= s_numerator;

  // We also want to take into account the diameter of the cylinders:
  // dim is the closest distance the two centerline of the cylinder can come.
  // If H was on the line B-C we would subtract dim/|BC| from s;
  // since H is closer from A, we multiply this ratio by |AB|/|AC|.

  double correction_term = dim * Matrix::norm(vec_1) / (Matrix::norm(vec_3) * Matrix::norm(vec_4));

  s -= correction_term;
  if (s < 0) {
    s = 0;
  }

  return s;
}

void CollisionCheck::addPhysicalBondIfCrossing(const std::array<double, 3>& A, const std::array<double, 3>& B,
                                               const std::array<double, 3>& C, PhysicalCylinder* moving,
                                               PhysicalCylinder* still) {
  //the immobile end of the moving cylinder

  // the distal end of the immobile cylinder
  auto D = still->getMassLocation();
  // the proximal end of the cylinder
  auto E = still->proximalEnd();

  std::array<double, 3> vec_1, vec_2, vec_3, n, vec_normal_a, vec_normal_b, vec_normal_c;
  vec_1 = Matrix::subtract(B, A);
  vec_2 = Matrix::subtract(C, A);
  vec_3 = Matrix::subtract(C, B);
  n = Matrix::crossProduct(vec_1, vec_2);
  // plane through A and perpendicular to vecNormal (= plane through A,B and C):
  // (good summary on planes at http://mathworld.wolfram.com/Plane.html ).
  // Basically, the  plane perpendicular to n = (a,b,c) and containing the point Xo = (xo,yo,zo)
  // contains the points X = (x,y,z) satisfying : n dot (X-Xo) = 0
  // This last equation gives a*x + b*y + c*z = d, with d = a*xo + b*yo + c*zo
  // Here we take A as our point Xo.
  double a, b, c, d;
  a = n[0];
  b = n[1];
  c = n[2];
  d = Matrix::dot(A, n);
  // H = intersection point of previous plane and line DE
  // line = E + lambda(D-E). We look for a lambda s.t for each component
  // the equation of the previous plane is satified
  double lambda = (d - (a * E[0] + b * E[1] + c * E[2])) / (a * (D[0] - E[0]) + b * (D[1] - E[1]) + c * (D[2] - E[2]));
  // Is the intersection point between D and E ?
  if (lambda > 1 || lambda < 0) {
    return;
  }
  auto H = Matrix::add(E, Matrix::scalarMult(lambda, Matrix::subtract(D, E)));
  // Is H inside the triangle ABC? yes if it satisfies three conditions:
  // i) H and C are on the same side of vec_1
  auto side_1 = Matrix::subtract(H, A);
  auto side_2 = Matrix::subtract(C, A);
  vec_normal_a = Matrix::normalize(Matrix::crossProduct(n, vec_1));
  if (STLUtil::sgn(Matrix::dot(vec_normal_a, side_1) * Matrix::dot(vec_normal_a, side_2)) < 0)
    return;
  // ii) H and B are on the same side of vec_2
  side_1 = Matrix::subtract(H, A);
  side_2 = Matrix::subtract(B, A);
  vec_normal_b = Matrix::crossProduct(n, vec_2);
  if (STLUtil::sgn(Matrix::dot(vec_normal_b, side_1) * Matrix::dot(vec_normal_b, side_2)) < 0)
    return;
  // iii) H and A are on the same side of vec_3
  side_1 = Matrix::subtract(H, B);
  side_2 = Matrix::subtract(A, B);
  vec_normal_c = Matrix::crossProduct(n, vec_3);
  if (STLUtil::sgn(Matrix::dot(vec_normal_c, side_1) * Matrix::dot(vec_normal_c, side_2)) < 0)
    return;

  // If we arrive here it means that the cylinder D-E is inside the area swaped by the moving cylinder.
  // and this means either that something illegal happened (and in this case, as correction, we put a PhysicalBond to force
  // the branches to take their normal configuration later) or that a former forbidden situation has just been corrected
  // (and in this case, we remove the PhysicalBond between theses objects).

  bool already_physical_bond = false;
  for (auto pb_on_still : still->getPhysicalBonds()) {
    if (pb_on_still->getOppositePhysicalObject(still) == moving) {
      already_physical_bond = true;
      pb_on_still->vanish();
    }
  }

  if (!already_physical_bond) {
    auto p = PhysicalBond::create(still, std::array<double, 2> { lambda * Matrix::distance(E, D), 0 }, moving,
                                  std::array<double, 2> { 0, 0 }, -10, 10);
    p->setSlidingAllowed(true);
  }
}

}  // namespace physics
}  // namespace cx3d
