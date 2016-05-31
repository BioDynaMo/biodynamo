#ifndef PHYSICS_DEBUG_PHYSICAL_SPHERE_DEBUG_H_
#define PHYSICS_DEBUG_PHYSICAL_SPHERE_DEBUG_H_

#include "string_util.h"
#include "physics/physical_sphere.h"

namespace cx3d {
namespace physics {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
class PhysicalSphereDebug : public PhysicalSphere {
 public:
  PhysicalSphereDebug()
      : PhysicalSphere() {
//    logConstrParameterless("PhysicalSphere");
//    std::cout << "DBG diameter: " << diameter_ << std::endl;
  }

  double getInterObjectForceCoefficient() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getInterObjectForceCoefficient();
    logReturn(ret);
    return ret;
  }

  void setInterObjectForceCoefficient(double coefficient) {
    logCall(coefficient);
    PhysicalSphere::setInterObjectForceCoefficient(coefficient);
    logReturnVoid();
  }

  double getRotationalInertia() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getRotationalInertia();
    logReturn(ret);
    return ret;
  }

  void setRotationalInertia(double rotational_inertia) {
    logCall(rotational_inertia);
    PhysicalSphere::setRotationalInertia(rotational_inertia);
    logReturnVoid();
  }

  bool isAPhysicalSphere() const {
    logCallParameterless();
    auto ret = PhysicalSphere::isAPhysicalSphere();
    logReturn(ret);
    return ret;
  }

  void movePointMass(double speed, const std::array<double, 3>& direction) {
    logCall(speed, direction);
    PhysicalSphere::movePointMass(speed, direction);
    logReturnVoid();
  }

  std::array<double, 3> originOf(PhysicalObject* daughter) {
    logCall(daughter);
    auto ret = PhysicalSphere::originOf(daughter);
    logReturn(ret);
    return ret;

  }

  SomaElement* getSomaElement() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getSomaElement();
    logReturn(ret);
    return ret;
  }

  void setSomaElement(SomaElement* soma) {
    logCall(soma);
    PhysicalSphere::setSomaElement(soma);
    logReturnVoid();
  }

  void changeVolume(double speed) {
    logCall(speed);
    PhysicalSphere::changeVolume(speed);
    logReturnVoid();
  }

  void changeDiameter(double speed) {
    logCall(speed);
    PhysicalSphere::changeDiameter(speed);
    logReturnVoid();
  }

  PhysicalCylinder::UPtr addNewPhysicalCylinder(double new_length, double phi, double theta) {
    logCall(new_length, phi, theta);
    auto ret = PhysicalSphere::addNewPhysicalCylinder(new_length, phi, theta);
    logReturn(ret.get());
    return std::move(ret);
  }

  PhysicalSphere::UPtr divide(double vr, double phi, double theta) {
    logCall(vr, phi, theta);
    auto ret = PhysicalSphere::divide(vr, phi, theta);
    logReturn(ret.get());
    return std::move(ret);
  }

  bool isInContactWithSphere(PhysicalSphere* s) {
    logCall(s);
    auto ret = PhysicalSphere::isInContactWithSphere(s);
    logReturn(ret);
    return ret;
  }

  bool isInContactWithCylinder(PhysicalCylinder* c) {
    logCall(c);
    auto ret = PhysicalSphere::isInContactWithCylinder(c);
    logReturn(ret);
    return ret;
  }

  std::array<double, 4> getForceOn(PhysicalCylinder* c) {
    logCall(c);
    auto ret = PhysicalSphere::getForceOn(c);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> getForceOn(PhysicalSphere* s) {
    logCall(s);
    auto ret = PhysicalSphere::getForceOn(s);
    logReturn(ret);
    return ret;
  }

  void runPhysics() {
    logCallParameterless();
    PhysicalSphere::runPhysics();
    logReturnVoid();
  }

  std::array<double, 3> getAxis() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getAxis();
    logReturn(ret);
    return ret;
  }

  std::list<PhysicalCylinder*> getDaughters() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getDaughters();
    logReturn(ret);
    return ret;
  }

  void runIntracellularDiffusion() {
    logCallParameterless();
    PhysicalSphere::runIntracellularDiffusion();
    logReturnVoid();
  }

  std::array<double, 3> transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesGlobalToLocal(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesLocalToGlobal(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesLocalToPolar(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesPolarToLocal(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesPolarToGlobal(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::transformCoordinatesGlobalToPolar(position);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& position) const {
    logCall(position);
    auto ret = PhysicalSphere::getUnitNormalVector(position);
    logReturn(ret);
    return ret;
  }

  CellElement* getCellElement() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getCellElement();
    logReturn(ret);
    return ret;
  }

  bool isRelative(PhysicalObject* po) const {
    logCall(po);
    auto ret = PhysicalSphere::isRelative(po);
    logReturn(ret);
    return ret;
  }

  double getLength() const {
    logCallParameterless();
    auto ret = PhysicalSphere::getLength();
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> forceTransmittedFromDaugtherToMother(PhysicalObject* mother) {
    logCall(mother);
    auto ret = PhysicalSphere::forceTransmittedFromDaugtherToMother(mother);
    logReturn(ret);
    return ret;
  }

  void removeDaughter(PhysicalObject* daughter) {
    logCall(daughter);
    PhysicalSphere::removeDaugther(daughter);
    logReturnVoid();
  }

  void updateRelative(PhysicalObject* old_relative, PhysicalObject* new_relative) {
    logCall(old_relative, new_relative);
    PhysicalSphere::updateRelative(old_relative, new_relative);
    logReturnVoid();
  }

  void updateDependentPhysicalVariables() {
    logCallParameterless();
    PhysicalSphere::updateDependentPhysicalVariables();
    logReturnVoid();
  }

//  void updateIntracellularConcentrations() {
//    logCallParameterless();
//    PhysicalSphere::updateIntracellularConcentrations();
//    logReturnVoid();
//  }

//  void updateVolume() {
//    logCallParameterless();
//    PhysicalSphere::updateVolume();
//    logReturnVoid();
//  }

  void updateDiameter() {
    logCallParameterless();
    PhysicalSphere::updateDiameter();
    logReturnVoid();
  }

 private:
  PhysicalSphereDebug(const PhysicalSphereDebug&) = delete;
  PhysicalSphereDebug& operator=(const PhysicalSphereDebug&) = delete;
};

}
// physics
}// cx3d

#endif // PHYSICS_DEBUG_PHYSICAL_SPHERE_DEBUG_H_
