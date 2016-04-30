#ifndef PHYSICS_DEBUG_PHYSICAL_CYLINDER_DEBUG_H_
#define PHYSICS_DEBUG_PHYSICAL_CYLINDER_DEBUG_H_

#include "string_util.h"
#include "physics/physical_cylinder.h"

namespace cx3d {
namespace physics {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
class PhysicalCylinderDebug : public PhysicalCylinder {
 public:
  PhysicalCylinderDebug()
      : PhysicalCylinder() {
//    logConstrParameterless("PhysicalSphere");
  }

  virtual std::array<double, 3> getAxis() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getAxis();
    logReturn(ret);
    return ret;
  }

  virtual std::shared_ptr<PhysicalCylinder> getCopy() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getCopy();
    logReturn(ret);
    return ret;
  }

  virtual bool isRelative(const std::shared_ptr<PhysicalObject>& po) const override {
    logCall(po);
    auto ret = PhysicalCylinder::isRelative(po);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> originOf(const std::shared_ptr<PhysicalObject>& daughter) override {
    logCall(daughter);
    auto ret = PhysicalCylinder::originOf(daughter);
    logReturn(ret);
    return ret;
  }

  virtual void removeDaugther(const std::shared_ptr<PhysicalObject>& daughter) override {
    logCall(daughter);
    PhysicalCylinder::removeDaugther(daughter);
    logReturnVoid();
  }

  virtual void updateRelative(const std::shared_ptr<PhysicalObject>& old_relative,
      const std::shared_ptr<PhysicalObject>& new_Relative) override {
    logCall(old_relative, new_Relative);
    PhysicalCylinder::updateRelative(old_relative, new_Relative);
    logReturnVoid();
  }

  virtual std::array<double, 3> forceTransmittedFromDaugtherToMother(const std::shared_ptr<PhysicalObject>& mother) override {
    logCall(mother);
    auto ret = PhysicalCylinder::forceTransmittedFromDaugtherToMother(mother);
    logReturn(ret);
    return ret;
  }

  virtual bool runDiscretization() override {
    logCallParameterless();
    auto ret = PhysicalCylinder::runDiscretization();
    logReturn(ret);
    return ret;
  }

  virtual void updateSpatialOrganizationNodePosition() override {
    logCallParameterless();
    PhysicalCylinder::updateSpatialOrganizationNodePosition();
    logReturnVoid();
  }

  virtual void extendCylinder(double speed, const std::array<double, 3>& direction) override {
    logCall(speed, direction);
    PhysicalCylinder::extendCylinder(speed, direction);
    logReturnVoid();
  }

  virtual void movePointMass(double speed, const std::array<double, 3>& direction) override {
    logCall(speed, direction);
    PhysicalCylinder::movePointMass(speed, direction);
    logReturnVoid();
  }

  virtual bool retractCylinder(double speed) override {
    logCall(speed);
    auto ret = PhysicalCylinder::retractCylinder(speed);
    logReturn(ret);
    return ret;
  }

  virtual std::array<std::shared_ptr<PhysicalCylinder>, 2> bifurcateCylinder(double length,
      const std::array<double, 3>& direction_1,
      const std::array<double, 3>& direction_2) override {
    logCall(length, direction_1, direction_2);
    auto ret = PhysicalCylinder::bifurcateCylinder(length, direction_1, direction_2);
    logReturn(ret);
    return ret;
  }

  virtual std::shared_ptr<PhysicalCylinder> branchCylinder(double length, const std::array<double, 3>& direction) override {
    logCall(length, direction);
    auto ret = PhysicalCylinder::branchCylinder(length, direction);
    logReturn(ret);
    return ret;
  }

  virtual void setRestingLengthForDesiredTension(double tension) override {
    logCall(tension);
    PhysicalCylinder::setRestingLengthForDesiredTension(tension);
    logReturnVoid();
  }

  virtual void changeVolume(double speed) override {
    logCall(speed);
    PhysicalCylinder::changeVolume(speed);
    logReturnVoid();
  }

  virtual void changeDiameter(double speed) override {
    logCall(speed);
    PhysicalCylinder::changeDiameter(speed);
    logReturnVoid();
  }

  virtual void runPhysics() override {
    logCallParameterless();
    PhysicalCylinder::runPhysics();
    logReturnVoid();
  }

  virtual std::array<double, 3> getForceOn(const std::shared_ptr<PhysicalSphere>& s) override {
    logCall(s);
    auto ret = PhysicalCylinder::getForceOn(s);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 4> getForceOn(const std::shared_ptr<PhysicalCylinder>& c) override {
    logCall(c);
    auto ret = PhysicalCylinder::getForceOn(c);
    logReturn(ret);
    return ret;
  }

  virtual bool isInContactWithSphere(const std::shared_ptr<PhysicalSphere>& s) override {
    logCall(s);
    auto ret = PhysicalCylinder::isInContactWithSphere(s);
    logReturn(ret);
    return ret;
  }

  virtual bool isInContactWithCylinder(const std::shared_ptr<PhysicalCylinder>& c) override {
    logCall(c);
    auto ret = PhysicalCylinder::isInContactWithCylinder(c);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> closestPointTo(const std::array<double, 3>& p) override {
    logCall(p);
    auto ret = PhysicalCylinder::closestPointTo(p);
    logReturn(ret);
    return ret;
  }

  virtual void runIntracellularDiffusion() override {
    logCallParameterless();
    PhysicalCylinder::runIntracellularDiffusion();
    logReturnVoid();
  }

  virtual std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::getUnitNormalVector(position);
    logReturn(ret);
    return ret;
  }

  virtual void updateLocalCoordinateAxis() override {
    logCallParameterless();
    PhysicalCylinder::updateLocalCoordinateAxis();
    logReturnVoid();
  }

  virtual void updateDependentPhysicalVariables() override {
    logCallParameterless();
    PhysicalCylinder::updateDependentPhysicalVariables();
    logReturnVoid();
  }

  virtual void updateDiameter() override {
    logCallParameterless();
    PhysicalCylinder::updateDiameter();
    logReturnVoid();
  }

//  virtual void updateVolume() override {
//    logCallParameterless();
//    PhysicalCylinder::updateVolume();
//    logReturnVoid();
//  }
//
//  virtual void updateIntracellularConcentrations() override {
//    logCallParameterless();
//    PhysicalCylinder::updateIntracellularConcentrations();
//    logReturnVoid();
//  }

  virtual std::array<double, 3> transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesGlobalToLocal(position);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesLocalToGlobal(position);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesLocalToPolar(position);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesPolarToLocal(position);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesPolarToGlobal(position);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = PhysicalCylinder::transformCoordinatesGlobalToPolar(position);
    logReturn(ret);
    return ret;
  }

  virtual std::shared_ptr<local_biology::CellElement> getCellElement() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getCellElement();
    logReturn(ret);
    return ret;
  }

  virtual std::shared_ptr<local_biology::NeuriteElement> getNeuriteElement() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getNeuriteElement();
    logReturn(ret);
    return ret;
  }

  virtual void setNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite) override {
    logCall(neurite);
    PhysicalCylinder::setNeuriteElement(neurite);
    logReturnVoid();
  }

  virtual std::shared_ptr<PhysicalCylinder> getDaughterLeft() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getDaughterLeft();
    logReturn(ret);
    return ret;
  }

  virtual std::shared_ptr<PhysicalCylinder> getDaughterRight() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getDaughterRight();
    logReturn(ret);
    return ret;
  }

//  virtual std::shared_ptr<PhysicalObject> getMother() const override {
//    logCallParameterless();
//    auto ret = PhysicalCylinder::getMother();
//    logReturn(ret);
//    return ret;
//  }

  virtual void setMother(const std::shared_ptr<PhysicalObject>& mother) override {
    logCall(mother);
    PhysicalCylinder::setMother(mother);
    logReturnVoid();
  }

  virtual void setDaughterLeft(const std::shared_ptr<PhysicalCylinder>& daughter) override {
    logCall(daughter);
    PhysicalCylinder::setDaughterLeft(daughter);
    logReturnVoid();
  }

  virtual void setDaughterRight(const std::shared_ptr<PhysicalCylinder>& daughter) override {
    logCall(daughter);
    PhysicalCylinder::setDaughterRight(daughter);
    logReturnVoid();
  }

//  virtual void setBranchOrder(int branchOrder) override {
//    logCall(branchOrder);
//    PhysicalCylinder::setBranchOrder(branchOrder);
//    logReturnVoid();
//  }

  virtual int getBranchOrder() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getBranchOrder();
    logReturn(ret);
    return ret;
  }

  virtual double getActualLength() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getActualLength();
    logReturn(ret);
    return ret;
  }

  virtual void setActualLength(double actual_length) override {
    logCall(actual_length);
    PhysicalCylinder::setActualLength(actual_length);
    logReturnVoid();
  }

  virtual double getRestingLength() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getRestingLength();
    logReturn(ret);
    return ret;
  }

  virtual void setRestingLength(double resting_length) override {
    logCall(resting_length);
    PhysicalCylinder::setRestingLength(resting_length);
    logReturnVoid();
  }

  virtual std::array<double, 3> getSpringAxis() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getSpringAxis();
    logReturn(ret);
    return ret;
  }

  virtual void setSpringAxis(const std::array<double, 3>& axis) override {
    logCall(axis);
    PhysicalCylinder::setSpringAxis(axis);
    logReturnVoid();
  }

  virtual double getSpringConstant() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getSpringConstant();
    logReturn(ret);
    return ret;
  }

//  virtual void setSpringConstant(double springConstant) override {
//    logCall(springConstant);
//    PhysicalCylinder::setSpringConstant(springConstant);
//    logReturnVoid();
//  }

  virtual double getTension() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getTension();
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> getUnitaryAxisDirectionVector() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getUnitaryAxisDirectionVector();
    logReturn(ret);
    return ret;
  }

  virtual bool isTerminal() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::isTerminal();
    logReturn(ret);
    return ret;
  }

  virtual bool bifurcationPermitted() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::bifurcationPermitted();
    logReturn(ret);
    return ret;
  }

  virtual bool branchPermitted() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::branchPermitted();
    logReturn(ret);
    return ret;
  }

//  virtual std::array<double, 3> proximalEnd() const override {
//    logCallParameterless();
//    auto ret = PhysicalCylinder::proximalEnd();
//    logReturn(ret);
//    return ret;
//  }
//
//  virtual std::array<double, 3> distalEnd() const override {
//    logCallParameterless();
//    auto ret = PhysicalCylinder::distalEnd();
//    logReturn(ret);
//    return ret;
//  }

  virtual double lengthToProximalBranchingPoint() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::lengthToProximalBranchingPoint();
    logReturn(ret);
    return ret;
  }

//  virtual bool isAPhysicalCylinder() const override {
//    logCallParameterless();
//    auto ret = PhysicalCylinder::isAPhysicalCylinder();
//    logReturn(ret);
//    return ret;
//  }

  virtual double getLength() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getLength();
    logReturn(ret);
    return ret;
  }

  virtual double getInterObjectForceCoefficient() const override {
    logCallParameterless();
    auto ret = PhysicalCylinder::getInterObjectForceCoefficient();
    logReturn(ret);
    return ret;
  }

  virtual void setInterObjectForceCoefficient(double coefficient) override {
    logCall(coefficient);
    PhysicalCylinder::setInterObjectForceCoefficient(coefficient);
    logReturnVoid();
  }

private:
  PhysicalCylinderDebug(const PhysicalCylinderDebug&) = delete;
  PhysicalCylinderDebug& operator=(const PhysicalCylinderDebug&) = delete;
};

}  // physics
}  // cx3d

#endif // PHYSICS_DEBUG_PHYSICAL_CYLINDER_DEBUG_H_
