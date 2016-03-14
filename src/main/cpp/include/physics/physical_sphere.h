#ifndef PHYSICS_PHYSICAL_SPHERE_H_
#define PHYSICS_PHYSICAL_SPHERE_H_

#include <array>
#include <exception>

#include "physics/physical_object.h"

namespace cx3d {
namespace physics {

class PhysicalSphere : public PhysicalObject {
 public:
  virtual ~PhysicalSphere() {
  }

  virtual std::array<double, 3> getMassLocation() {
    throw std::logic_error(
        "PhysicalSphere::getMassLocation must not be called - Java must provide implementation at this point");
  }

  virtual double getDiameter() {
    throw std::logic_error(
        "PhysicalSphere::getDiameter must not be called - Java must provide implementation at this point");
  }

  virtual double getInterObjectForceCoefficient() {
    throw std::logic_error(
        "PhysicalSphere::getInterObjectForceCoefficient must not be called - Java must provide implementation at this point");
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_SPHERE_H_
