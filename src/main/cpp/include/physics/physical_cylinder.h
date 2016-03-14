#ifndef PHYSICS_PHYSICAL_CYLINDER_H_
#define PHYSICS_PHYSICAL_CYLINDER_H_

#include <array>
#include <exception>

#include "physics/physical_object.h"

namespace cx3d {
namespace physics {

class PhysicalCylinder : public PhysicalObject {
 public:
  virtual ~PhysicalCylinder(){
  }

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

  virtual std::array<double, 3> proximalEnd() {
    throw std::logic_error(
        "PhysicalCylinder::proximalEnd - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> distalEnd() {
    throw std::logic_error(
        "PhysicalCylinder::distalEnd - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> getSpringAxis() {
    throw std::logic_error(
        "PhysicalCylinder::getSpringAxis - must never be called - Java should provide implementation at this point");
  }

  virtual double getDiameter() {
    throw std::logic_error(
        "PhysicalCylinder::getSpringAxis - must never be called - Java should provide implementation at this point");
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_CYLINDER_H_
