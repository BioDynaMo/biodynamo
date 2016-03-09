#ifndef PHYSICS_PHYSICAL_CYLINDER_H_
#define PHYSICS_PHYSICAL_CYLINDER_H_

#include <exception>

#include "physics/physical_object.h"

namespace cx3d {
namespace physics {

class PhysicalCylinder : public PhysicalObject {
 public:
  virtual double getActualLength() {
    throw std::logic_error(
        "PhysicalCylinder::getActualLength - must never be called - Java should provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalCylinder> getDaughterLeft() {
    throw std::logic_error(
        "PhysicalCylinder::getDaughterLeft - must never be called - Java should provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalObject> getMother() {
    throw std::logic_error(
        "PhysicalCylinder::getMother - must never be called - Java should provide implementation at this point");
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_CYLINDER_H_
