#ifndef PHYSICS_INTER_OBJECT_FORCE_H_
#define PHYSICS_INTER_OBJECT_FORCE_H_

#include <string>

#include "sim_state_serializable.h"
#include "physics/inter_object_force.h"

namespace bdm {
namespace physics {

/**
 * Defines the 3D physical interactions between physical objects (cylinders and spheres).
 */
class DefaultForce : public InterObjectForce {
 public:
  DefaultForce();

  virtual ~DefaultForce();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /**
   * Force felt by a sphere (sphere1) due to the presence of another sphere (sphere2)
   * @param sphere_1
   * @param sphere_2
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromASphere(PhysicalSphere* sphere_1, PhysicalSphere* sphere_2) const
      override;

  /**
   * Force felt by a cylinder due to the presence of a sphere
   * @param cylinder
   * @param sphere
   * @return the 3 first elements represent the force exerted by the sphere onto the cylinder,
   * the fourth -when it exists- is the proportion of the force that is transmitted to the proximal end
   * (= the point mass of the mother).
   *
   */
  virtual std::array<double, 4> forceOnACylinderFromASphere(PhysicalCylinder* cylinder, PhysicalSphere* sphere) const
      override;

  /**
   * Force felt by sphere due to the presence of a cylinder
   * @param sphere
   * @param cylinder
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromACylinder(PhysicalSphere* sphere, PhysicalCylinder* cylinder) const
      override;

  /**
   * Force felt by a cylinder (cylinder1) due to the presence of another cylinder (cylinder2)
   * @param cylinder1
   * @param cylinder2
   * @return the 3 first elements represent the force exerted by cylinder2 onto cylinder1,
   * the fourth -when it exists- is the proportion of the force that is transmitted to the proximal end
   * of cylinder1 (= the point mass of the mother).
   */
  virtual std::array<double, 4> forceOnACylinderFromACylinder(PhysicalCylinder* cylinder1,
                                                              PhysicalCylinder* cylinder2) const override;

  virtual std::string toString() const;

 private:
  DefaultForce(const DefaultForce&) = delete;
  DefaultForce& operator=(const DefaultForce&) = delete;

  std::array<double, 4> computeForceOfASphereOnASphere(const std::array<double, 3>& c1, double r1,
                                                       const std::array<double, 3>& c2, double r2) const;
};

}  //namespace physics
}  //namespace bdm

#endif  // PHYSICS_INTER_OBJECT_FORCE_H_
