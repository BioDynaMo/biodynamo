#ifndef PHYSICS_PHYSICAL_CYLINDER_H_
#define PHYSICS_PHYSICAL_CYLINDER_H_

#include <array>
#include <exception>

#include "physics/physical_object.h"

namespace cx3d {
namespace physics {

class PhysicalCylinder : public PhysicalObject {
 public:
  virtual ~PhysicalCylinder() {
  }

  virtual void setActualLength(double length) {
    throw std::logic_error(
        "PhysicalCylinder::setActualLength - must never be called - Java should provide implementation at this point");
  }

  virtual double getActualLength() const {
    throw std::logic_error(
        "PhysicalCylinder::getActualLength - must never be called - Java should provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalCylinder> getDaughterLeft() const {
    throw std::logic_error(
        "PhysicalCylinder::getDaughterLeft - must never be called - Java should provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalObject> getMother() const {
    throw std::logic_error(
        "PhysicalCylinder::getMother - must never be called - Java should provide implementation at this point");
  }

  virtual void setMother(const std::shared_ptr<PhysicalObject>& mother) {
    throw std::logic_error(
        "PhysicalCylinder::setMother - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> proximalEnd() {
    throw std::logic_error(
        "PhysicalCylinder::proximalEnd - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> distalEnd() {
    throw std::logic_error(
        "PhysicalCylinder::distalEnd - must never be called - Java should provide implementation at this point");
  }

  virtual std::array<double, 3> getSpringAxis() const {
    throw std::logic_error(
        "PhysicalCylinder::getSpringAxis - must never be called - Java should provide implementation at this point");
  }

  virtual void setSpringAxis(const std::array<double, 3>& axis) {
    throw std::logic_error(
        "PhysicalCylinder::setSpringAxis - must never be called - Java should provide implementation at this point");
  }

  virtual void setRestingLengthForDesiredTension(double resting_length) {
    throw std::logic_error(
        "PhysicalCylinder::setRestingLengthForDesiredTension - must never be called - Java should provide implementation at this point");
  }

  virtual void updateLocalCoordinateAxis() {
    throw std::logic_error(
        "PhysicalCylinder::updateLocalCoordinateAxis - must never be called - Java should provide implementation at this point");
  }

  virtual std::string toString() const {
    throw std::logic_error(
            "PhysicalCylinder::toString - must never be called - Java should provide implementation at this point");
  }
};

struct PhysicalCylinderHash {
  std::size_t operator()(const std::shared_ptr<PhysicalCylinder>& element) const {
    return reinterpret_cast<std::size_t>(element.get());
  }
};

struct PhysicalCylinderEqual {
  bool operator()(const std::shared_ptr<PhysicalCylinder>& lhs, const std::shared_ptr<PhysicalCylinder>& rhs) const {
    return lhs.get() == rhs.get();
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_CYLINDER_H_
