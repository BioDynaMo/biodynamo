#ifndef PHYSICS_PHYSICAL_OBJECT_H_
#define PHYSICS_PHYSICAL_OBJECT_H_

#include <array>
#include <exception>

#include "physics/physical_node.h"

namespace cx3d {
namespace physics {

class PhysicalBond;

class PhysicalObject : public PhysicalNode {
 public:
  virtual ~PhysicalObject(){

  }

  virtual void removePhysicalBond(std::shared_ptr<PhysicalBond> bond) {
    throw std::logic_error(
        "PhysicalObject::removePhysicalBond - must never be called - Java should provide implementation at this point");
  }

  virtual void addPhysicalBond(std::shared_ptr<PhysicalBond> bond) {
    throw std::logic_error(
        "PhysicalObject::addPhysicalBond - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& coord) {
    throw std::logic_error(
        "PhysicalObject::transformCoordinatesPolarToGlobal - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 2> transformCoordinatesGlobalToPolar(const std::array<double, 3>& coord) {
    throw std::logic_error(
        "PhysicalObject::transformCoordinatesGlobalToPolar - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> getMassLocation() {
    throw std::logic_error(
        "PhysicalObject::getMassLocation - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> getXAxis() {
    throw std::logic_error(
        "PhysicalObject::getXAxis - must never be called - Java should provide implementation at this point");
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_OBJECT_H_
