#ifndef LOCAL_BIOLOGY_NEURITE_ELEMENT_H_
#define LOCAL_BIOLOGY_NEURITE_ELEMENT_H_

#include <list>
#include <memory>
#include <exception>

#include "cell_element.h"

namespace cx3d {

namespace physics {
class PhysicalObject;
class PhysicalCylinder;
}  // namespace physics

namespace local_biology {

class LocalBiologyModule;

class NeuriteElement : public CellElement {
 public:
  virtual ~NeuriteElement() {
  }

  virtual std::shared_ptr<NeuriteElement> getCopy() {
    throw std::logic_error(
        "NeuriteElement::getCopy must never be called - Java must provide implementation at this point");
  }

  virtual void removeYourself() {
    throw std::logic_error(
        "NeuriteElement::removeYourself must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalCylinder> getPhysicalCylinder() {
    throw std::logic_error(
        "NeuriteElement::getPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual void setPhysical(const std::shared_ptr<physics::PhysicalObject>& pc) {
    throw std::logic_error(
        "NeuriteElement::setPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<LocalBiologyModule> > getLocalBiologyModulesList() {
    throw std::logic_error(
        "NeuriteElement::getLocalBiologyModulesList must never be called - Java must provide implementation at this point");
  }

  virtual void addLocalBiologyModule(const std::shared_ptr<LocalBiologyModule>& module) {
    throw std::logic_error(
        "NeuriteElement::addLocalBiologyModule must never be called - Java must provide implementation at this point");
  }
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_NEURITE_ELEMENT_H_
