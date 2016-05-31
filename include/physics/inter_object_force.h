#ifndef PHYSICS_DEFAULT_FORCE_H_
#define PHYSICS_DEFAULT_FORCE_H_

#include <array>
#include <memory>
#include <exception>

namespace bdm {
namespace physics {

class PhysicalSphere;
class PhysicalCylinder;

/**
 * Defines the 3D physical interactions between physical objects (cylinders and spheres).
 */
class InterObjectForce : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<InterObjectForce>;

  InterObjectForce() {
  }

  virtual ~InterObjectForce() {
  }

  /**
   * Force felt by a sphere (sphere1) due to the presence of another sphere (sphere2)
   * @param sphere_1
   * @param sphere_2
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromASphere(PhysicalSphere* sphere_1, PhysicalSphere* sphere_2) const = 0;

  /**
   * Force felt by a cylinder due to the presence of a sphere
   * @param cylinder
   * @param sphere
   * @return the 3 first elements represent the force exerted by the sphere onto the cylinder,
   * the fourth -when it exists- is the proportion of the force that is transmitted to the proximal end
   * (= the point mass of the mother).
   *
   */
  virtual std::array<double, 4> forceOnACylinderFromASphere(PhysicalCylinder* cylinder,
                                                            PhysicalSphere* sphere) const = 0;

  /**
   * Force felt by sphere due to the presence of a cylinder
   * @param sphere
   * @param cylinder
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromACylinder(PhysicalSphere* sphere,
                                                            PhysicalCylinder* cylinder) const = 0;

  /**
   * Force felt by a cylinder (cylinder1) due to the presence of another cylinder (cylinder2)
   * @param cylinder1
   * @param cylinder2
   * @return the 3 first elements represent the force exerted by cylinder2 onto cylinder1,
   * the fourth -when it exists- is the proportion of the force that is transmitted to the proximal end
   * of cylinder1 (= the point mass of the mother).
   */
  virtual std::array<double, 4> forceOnACylinderFromACylinder(PhysicalCylinder* cylinder1,
                                                              PhysicalCylinder* cylinder2) const = 0;

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override = 0;
};

}  //namespace physics
}  //namespace bdm

#endif  // PHYSICS_DEFAULT_FORCE_H_
