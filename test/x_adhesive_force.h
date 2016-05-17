#ifndef TEST_X_ADHESIVE_FORCE_H_
#define TEST_X_ADHESIVE_FORCE_H_

#include <array>
#include <memory>

#include "java_util.h"
#include "physics/inter_object_force.h"
#include "physics/physical_sphere.h"
#include "physics/physical_cylinder.h"

namespace cx3d {

using physics::PhysicalSphere;
using physics::PhysicalCylinder;
using physics::InterObjectForce;

/**
 * Slightly modified from the default force, this class serves a InterObjectForce
 * in Figure 9;
 */
class XAdhesiveForce : public InterObjectForce {
 public:
  XAdhesiveForce(const std::shared_ptr<JavaUtil2>& java)
      : java_ { java } {
  }

  virtual ~XAdhesiveForce() {
  }
  XAdhesiveForce(const XAdhesiveForce&) = delete;
  XAdhesiveForce& operator=(const XAdhesiveForce&) = delete;

  std::array<double, 3> forceOnASphereFromASphere(PhysicalSphere* sphere1,
                                                  PhysicalSphere* sphere2) const override {
    // defining center and radius of spheres
    auto c_1 = sphere1->getMassLocation();
    double r_1 = 0.5 * sphere1->getDiameter();
    auto c_2 = sphere2->getMassLocation();
    double r_2 = 0.5 * sphere2->getDiameter();
    // We take virtual bigger radii to have a distant interaction, to get a desired density.
    r_1 += 1.5;
    r_2 += 1.5;
    // the 3 components of the vector c2 -> c1
    double comp_1 = c_1[0] - c_2[0];
    double comp_2 = c_1[1] - c_2[1];
    double comp_3 = c_1[2] - c_2[2];
    double distance_between_centers = java_->sqrt(comp_1 * comp_1 + comp_2 * comp_2 + comp_3 * comp_3);
    // the overlap distance (how much one penetrates in the other)
    double delta = r_1 + r_2 - distance_between_centers;
    // if no overlap : no force
    if (delta < 0)
      return {0,0,0};
    // to avoid a division by 0 if the centers are (almost) at the same location
    if (distance_between_centers < 0.00000001) {
      return java_->matrixRandomNoise3(3);;
    } else {
      // the force itself
      double r = (r_1 * r_2) / (r_1 + r_2);
      double gamma = 1;     // attraction coeff
      double k = 2;           // repulsion coeff
      double f = k * delta - gamma * java_->sqrt(r * delta);

      double module = f / distance_between_centers;
      return {module * comp_1, module * comp_2, module * comp_3};
    }
  }

  std::array<double, 4> forceOnACylinderFromASphere(PhysicalCylinder* cylinder,
                                                    PhysicalSphere* sphere) const override {
    // define some geometrical values
    auto p_p = cylinder->proximalEnd();
    auto p_d = cylinder->distalEnd();
    auto axis = cylinder->getSpringAxis();
    double actual_length = Matrix::norm(axis);
    double d = cylinder->getDiameter();
    auto c = sphere->getMassLocation();
    double r = 0.5 * sphere->getDiameter();

    // I. If the cylinder is small with respect to the sphere:
    // we only consider the interaction between the sphere and the point mass
    // (i.e. distal point) of the cylinder - that we treat as a sphere.
    if (actual_length < r) {
      auto tmp = computeForceOfASphereOnASphere(p_d, d * 0.5, c, r);
      return {tmp[0], tmp[1], tmp[2], 0.0};
    }

    // II. If the cylinder is of the same scale or bigger than the sphere,
    // we look at the interaction between the sphere and the closest point
    // (to the sphere center) on the cylinder. This interaction is distributed to
    // the two ends of the cylinder: the distal (point mass of the segment) and
    // the proximal (point mass of the mother of the segment).

    // 1)   Finding cc : the closest point to c on the line p_p p_d ("line" and not "segment")
    //    It is the projection of the vector p_p->c onto the vector p_p->p_d (=axis)
    auto p_pc = Matrix::subtract(c, p_p);

    //    projection of p_pc onto axis = (p_pc.axis)/norm(axis)^2  * axis
    //    length of the projection = (pPc.axis)/norm(axis)

    double p_pc_dot_axis = p_pc[0] * axis[0] + p_pc[1] * axis[1] + p_pc[2] * axis[2];
    double k = p_pc_dot_axis / (actual_length * actual_length);
    //    cc = pP + k* axis
    std::array<double, 3> cc { p_p[0] + k * axis[0], p_p[1] + k * axis[1], p_p[2] + k * axis[2] };

    // 2) Look if c -and hence cc- is (a) between pP and pD, (b) before pP or (c) after pD
    double proportion_transmited_to_proximal_end;
    if (k <= 1.0 && k >= 0.0) {
      //    a)  if cc (the closest point to c on the line pPpD) is between pP and pD
      //      the force is distributed to the two nodes
      proportion_transmited_to_proximal_end = 1.0 - k;
    } else if (k < 0) {
      //    b)  if the closest point to c on the line pPpD is before pP
      //      the force is only on the proximal end (the mother point mass)
      proportion_transmited_to_proximal_end = 1.0;
      cc = p_p;
    } else {     // if(K>1)
      //    c) if cc is after pD, the force is only on the distal end (the segment's point mass).
      proportion_transmited_to_proximal_end = 0.0;
      cc = p_d;
    }

    // 3)   If the smallest distance between the cylinder and the center of the sphere
    //    is larger than the radius of the two objects , there is no interaction:
    double penetration = d / 2 + r - Matrix::distance(c, cc);
    if (penetration <= 0) {
      return {0.0, 0.0, 0.0, 0.0};
    }
    auto force = computeForceOfASphereOnASphere(cc, d * 0.5, c, r);
    return {force[0], force[1], force[2], proportion_transmited_to_proximal_end};
  }

  std::array<double, 3> forceOnASphereFromACylinder(PhysicalSphere* sphere,
                                                    PhysicalCylinder* cylinder) const override {
    // it is the opposite of force on a cylinder from sphere:
    auto temp = forceOnACylinderFromASphere(cylinder, sphere);
    return {-temp[0], -temp[1], -temp[2]};
  }

  std::array<double, 4> forceOnACylinderFromACylinder(PhysicalCylinder* cylinder1,
                                                      PhysicalCylinder* cylinder2) const
                                                          override {
    // define some geometrical values
    auto a = cylinder1->proximalEnd();
    auto b = cylinder1->getMassLocation();
    double d_1 = cylinder1->getDiameter();
    auto c = cylinder2->proximalEnd();
    auto d = cylinder2->getMassLocation();
    double d_2 = cylinder2->getDiameter();

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

    std::array<double, 3> P1, P2;

    double denom = d2121 * d4343 - d4321 * d4321;

    //if the two segments are not ABSOLUTLY parallel
    if (denom > 0.000000000001) {
      double numer = d1343 * d4321 - d1321 * d4343;

      double mua = numer / denom;
      double mub = (d1343 + mua * d4321) / d4343;

      bool both_dutside = true;

      if (mua < 0) {
        P1 = a;
        k = 1;
      } else if (mua > 1) {
        P1 = b;
        k = 0;
      } else {
        both_dutside = false;
        P1 = {a[0]+mua*p21x, a[1]+mua*p21y, a[2]+mua*p21z};
        k = 1 - mua;
      }

      if (mub < 0) {
        P2 = c;
      } else if (mub > 1) {
        P2 = d;
      } else {
        both_dutside = false;
        P2 = {c[0]+mub*p43x, c[1]+mub*p43y, c[2]+mub*p43z};
      }

      // if twice mu outside of [0,1], we might have a problem.
      // for each vector I check if from my P to his P (PtoP), projected onto me is really outside the segment
      if (both_dutside) {
        // On the first Vector
        auto p1_to_p2 = Matrix::subtract(P2, P1);
        std::array<double, 3> p1_to_other_end;
        if (mua < 0) {  // P1 = A
          p1_to_other_end = Matrix::subtract(b, a);
        } else {  // P1 = B
          p1_to_other_end = Matrix::subtract(a, b);
        }

        auto projj = Matrix::projectionOnto(p1_to_p2, p1_to_other_end);

        if (Matrix::dot(projj, p1_to_other_end) > 0) {
          if (Matrix::norm(projj) < Matrix::norm(p1_to_other_end)) {
            P1 = Matrix::add(P1, projj);
          } else {
            return {0,0,0,0.5};
          }
        }
        // on the second vector
        auto p2_to_p1 = Matrix::subtract(P1, P2);
        std::array<double, 3> p2_to_other_end;
        if (mub < 0) {  // P1 = A
          p2_to_other_end = Matrix::subtract(d, c);
        } else {  // P1 = B
          p2_to_other_end = Matrix::subtract(c, d);
        }

        projj = Matrix::projectionOnto(p2_to_p1, p2_to_other_end);

        if (Matrix::dot(projj, p2_to_other_end) > 0) {
          if (Matrix::norm(projj) < Matrix::norm(p2_to_other_end)) {
            P2 = Matrix::add(P2, projj);
          } else {
            return {0,0,0,0.5};
          }
        }
      }

    } else {
      P1 = Matrix::add(a, Matrix::scalarMult(0.5, Matrix::subtract(b, a)));
      P2 = Matrix::add(c, Matrix::scalarMult(0.5, Matrix::subtract(d, c)));
    }

    // W put a virtual sphere on the two cylinders
    auto force = Matrix::scalarMult(10, computeForceOfASphereOnASphere(P1, d_1 + 0, P2, d_2 + 0));

    //only the force component perpendicular to the cylinder is used...
    force = Matrix::subtract(force, Matrix::projectionOnto(force, cylinder1->getSpringAxis()));

    return {force[0], force[1], force[2], k};
  }

  StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{}");
    return sb;
  }

  double getAttractionRange() const {
    return attraction_range_;
  }

  void setAttractionRange(double attraction_range) {
    attraction_range_ = attraction_range;
  }

  double getAttractionStrength() const {
    return attraction_strength_;
  }

  void setAttractionStrength(double attraction_strength) {
    attraction_strength_ = attraction_strength;
  }

 private:
  std::shared_ptr<JavaUtil2> java_;

  double attraction_range_ = 4;

  double attraction_strength_ = 1;

  // is only used by sphere-cylinder and cylinder-cylinder....
  std::array<double, 3> computeForceOfASphereOnASphere(const std::array<double, 3>& c1, double r1,
                                                       const std::array<double, 3>& c2, double r2) const {
    // the 3 components of the vector c2 -> c1
    double comp_1 = c1[0] - c2[0];
    double comp_2 = c1[1] - c2[1];
    double comp_3 = c1[2] - c2[2];
    double distance_between_centers = java_->sqrt(comp_1 * comp_1 + comp_2 * comp_2 + comp_3 * comp_3);
    // the overlap distance (how much one penetrates in the other)
    double a = r1 + r2 - distance_between_centers;
    // if no overlap : no force

    if (a < -attraction_range_) {
      return {0,0,0};
    }
    if (a < 0) {
      // the force is prop to the square of the inter-penetration distance and to the radii.
      double module = attraction_strength_ * a / distance_between_centers;
      return {module*comp_1, module*comp_2, module*comp_3};  //force 2 on 1
    }

    // to avoid a division by 0 if the centers are (almost) at the same location
    if (distance_between_centers < 0.00000001) {
      return java_->matrixRandomNoise3(3);
    } else {
      // the force is prop to the square of the interpentration distance and to the radii.
      double module = a / distance_between_centers;
      return {module*comp_1, module*comp_2, module*comp_3};
    }
  }
};

}  //namespace cx3d

#endif  // TEST_X_ADHESIVE_FORCE_H_
