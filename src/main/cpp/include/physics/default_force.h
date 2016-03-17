#ifndef PHYSICS_INTER_OBJECT_FORCE_H_
#define PHYSICS_INTER_OBJECT_FORCE_H_

#include <string>

#include "java_util.h"
#include "sim_state_serializable.h"
#include "physics/inter_object_force.h"

namespace cx3d {
namespace physics {

/**
 * Defines the 3D physical interactions between physical objects (cylinders and spheres).
 */
class DefaultForce : public InterObjectForce {
 public:
  static void setJavaUtil(std::shared_ptr<JavaUtil2> java) {
    java_ = java;
  }

  static std::shared_ptr<DefaultForce> create();

  DefaultForce();

  virtual ~DefaultForce() {

  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /**
   * Force felt by a sphere (sphere1) due to the presence of another sphere (sphere2)
   * @param sphere_1
   * @param sphere_2
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromASphere(const std::shared_ptr<PhysicalSphere>& sphere_1,
                                                          const std::shared_ptr<PhysicalSphere>& sphere_2) const
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
  virtual std::array<double, 4> forceOnACylinderFromASphere(const std::shared_ptr<PhysicalCylinder>& cylinder,
                                                            const std::shared_ptr<PhysicalSphere>& sphere) const
                                                                override;

  /**
   * Force felt by sphere due to the presence of a cylinder
   * @param sphere
   * @param cylinder
   * @return
   */
  virtual std::array<double, 3> forceOnASphereFromACylinder(const std::shared_ptr<PhysicalSphere>& sphere,
                                                            const std::shared_ptr<PhysicalCylinder>& cylinder) const
                                                                override;

  /**
   * Force felt by a cylinder (cylinder1) due to the presence of another cylinder (cylinder2)
   * @param cylinder1
   * @param cylinder2
   * @return the 3 first elements represent the force exerted by cylinder2 onto cylinder1,
   * the fourth -when it exists- is the proportion of the force that is transmitted to the proximal end
   * of cylinder1 (= the point mass of the mother).
   */
  virtual std::array<double, 4> forceOnACylinderFromACylinder(const std::shared_ptr<PhysicalCylinder>& cylinder1,
                                                              const std::shared_ptr<PhysicalCylinder>& cylinder2) const
                                                                  override;

  virtual std::string toString() const;

  virtual bool equalTo(const std::shared_ptr<DefaultForce>& other) const;

 private:
  DefaultForce(const DefaultForce&) = delete;
  DefaultForce& operator=(const DefaultForce&) = delete;

  static std::shared_ptr<JavaUtil2> java_;

  std::array<double, 4> computeForceOfASphereOnASphere(const std::array<double, 3>& c1, double r1,
                                                       const std::array<double, 3>& c2, double r2) const;
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_INTER_OBJECT_FORCE_H_
