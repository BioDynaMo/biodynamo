#ifndef SIMULATION_ECM_H_
#define SIMULATION_ECM_H_

#include <iostream>
#include <array>
#include <string>
#include <memory>
#include <exception>
#include <list>

#include "color.h"

namespace cx3d {

namespace local_biology {
class SomaElement;
class NeuriteElement;
}  // namespace local_biology

namespace synapse {
class PhysicalSpine;
class PhysicalBouton;
}  // namespace synapse

namespace cells {
class Cell;
}  // namespace cells

namespace spatial_organization {
template<class T> class SpaceNode;
}  // namespace spatial_organization

namespace physics {

class Substance;
class IntracellularSubstance;
class PhysicalSphere;
class PhysicalCylinder;
class PhysicalObject;
class PhysicalBond;
class PhysicalNode;

}  // namespace physics

namespace simulation {

class ECM {
 public:
  virtual ~ECM() {
  }

  /** Returns true if some artificial gradient (of any sorts) have been defined.*/
  virtual bool thereAreArtificialGradients() {
    throw std::logic_error(
        "ECM::thereAreArtificialGradients must never be called - Java must provide implementation at this point");
  }

  /**
   * Gets the value of a chemical, at a specific position in space
   * @param nameOfTheChemical
   * @param position the location [x,y,z]
   * @return
   */
  virtual double getValueArtificialConcentration(const std::string& nameOfTheChemical,
                                                 const std::array<double, 3>& position) {
    throw std::logic_error(
        "ECM::getValueArtificialConcentration must never be called - Java must provide implementation at this point");
  }

  virtual double getECMtime() {
    throw std::logic_error("ECM::getECMtime must never be called - Java must provide implementation at this point");
  }

  /**
   * Gets the gradient of a chemical, at a specific altitude
   * @param nameOfTheChemical
   * @param position the location [x,y,z]
   * @return the gradient [dc/dx , dc/dy , dc/dz]
   */
  virtual std::array<double, 3> getGradientArtificialConcentration(const std::string& nameOfTheChemical,
                                                                   const std::array<double, 3>& position) {
    throw std::logic_error(
        "ECM::getGradientArtificialConcentration must never be called - Java must provide implementation at this point");
  }

  /** Returns an instance of <code>Substance</code>. If a similar substance (with the same id)
   * has already been declared as a template Substance, a copy of it is made (with
   * same id, degradation and diffusion constant, but concentration and quantity 0).
   * If it is the first time that this id is requested, a new template Substance is made
   * (with by-default values) and stored, and a copy will be returned.
   * @param id
   * @return new Substance instance
   */
  virtual std::shared_ptr<physics::Substance> substanceInstance(const std::string& id) {
    throw std::logic_error(
        "ECM::substanceInstance must never be called - Java must provide implementation at this point");
  }

  /** Returns an instance of <code>IntracellularSubstance</code>. If a similar
   * IntracellularSubstance (with the same id) has already been declared as a template
   * IntracellularSubstance, a copy of it is made (with same id, degradation constant,
   * diffusion constant, outside visibility and volume dependency, but concentration
   * and quantity 0).
   * If it is the first time that this id is requested, a new template IntracellularSubstance
   *  is made (with by-default values) and stored, and a copy will be returned.
   * @param id
   * @return new IntracellularSubstance instance
   */
  virtual std::shared_ptr<physics::IntracellularSubstance> intracellularSubstanceInstance(const std::string& id) {
    throw std::logic_error(
        "ECM::intracellularSubstanceInstance must never be called - Java must provide implementation at this point");
  }

  virtual void addPhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl) {
    throw std::logic_error(
        "ECM::addPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual void removePhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl) {
    throw std::logic_error(
        "ECM::removePhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual void addPhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& cyl) {
    throw std::logic_error(
        "ECM::addPhysicalSphere must never be called - Java must provide implementation at this point");
  }

  virtual void addNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite) {
    throw std::logic_error(
        "ECM::addNeuriteElement must never be called - Java must provide implementation at this point");
  }

  virtual void removeNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite) {
    throw std::logic_error(
        "ECM::removeNeuriteElement must never be called - Java must provide implementation at this point");
  }

  virtual bool getArtificialWallForSpheres() {
    throw std::logic_error(
        "ECM::getArtificialWallForSpheres must never be called - Java must provide implementation at this point");
  }

  virtual bool getArtificialWallForCylinders() {
    throw std::logic_error(
        "ECM::getArtificialWallForCylinders must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> forceFromArtificialWall(const std::array<double, 3>& location, double radius) {
    throw std::logic_error(
        "ECM::forceFromArtificialWall must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<local_biology::NeuriteElement> > getNeuriteElementList() {
    throw std::logic_error(
        "ECM::getNeuriteElementList must never be called - Java must provide implementation at this point");
  }

  virtual void addSomaElement(const std::shared_ptr<local_biology::SomaElement>& cyl) {
    throw std::logic_error(
        "ECM::addPhysicalSphere must never be called - Java must provide implementation at this point");
  }

  virtual void addCell(const std::shared_ptr<cells::Cell>& cell) {
    throw std::logic_error("ECM::addCell must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode> > getSpatialOrganizationNodeInstance(
      const std::array<double, 3>& position, const std::shared_ptr<physics::PhysicalNode>& ps) {
    throw std::logic_error(
        "ECM::getSpatialOrganizationNodeInstance must never be called - Java must provide implementation at this point");
  }

  virtual double getGaussianDouble(double mean, double standard_deviation) {
    throw std::logic_error(
        "ECM::getGaussianDouble must never be called - Java must provide implementation at this point");
  }

  virtual Color cellTypeColor(const std::string& type) {
    throw std::logic_error("ECM::cellTypeColor must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<physics::PhysicalNode>> getPhysicalNodeList() {
    throw std::logic_error(
        "ECM::getPhysicalNodeList must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<physics::PhysicalCylinder>> getPhysicalCylinderList() {
    throw std::logic_error(
        "ECM::getPhysicalCylinderList must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<physics::PhysicalSphere>> getPhysicalSphereList() {
    throw std::logic_error(
        "ECM::getPhysicalSphereList must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<local_biology::SomaElement>> getSomaElementList() {
    throw std::logic_error(
        "ECM::getSomaElementList must never be called - Java must provide implementation at this point");
  }

  virtual std::list<std::shared_ptr<cells::Cell>> getCellList() {
    throw std::logic_error("ECM::getCellList must never be called - Java must provide implementation at this point");
  }

  virtual void increaseECMtime(double dt) {
    throw std::logic_error(
        "ECM::increaseECMTime must never be called - Java must provide implementation at this point");
  }

  virtual void viewRepaint() {
    throw std::logic_error("ECM::viewRepaint must never be called - Java must provide implementation at this point");
  }

  virtual void createGUI() {
    throw std::logic_error("ECM::createView must never be called - Java must provide implementation at this point");
  }

  virtual double getRandomDouble1() {
    throw std::logic_error(
        "ECM::getRandomDouble must never be called - Java must provide implementation at this point");
  }

  virtual double matrixNextRandomDouble() {
    throw std::logic_error(
        "ECM::matrixNextRandomDouble must never be called - Java must provide implementation at this point");
  }

  virtual double exp(double d) {
    throw std::logic_error("ECM::exp must never be called - Java must provide implementation at this point");
  }

  virtual double cbrt(double d) {
    throw std::logic_error("ECM::cbrt must never be called - Java must provide implementation at this point");
  }

  virtual double sqrt(double d) {
    throw std::logic_error("ECM::sqrt must never be called - Java must provide implementation at this point");
  }

  virtual double cos(double d) {
    throw std::logic_error("ECM::cos must never be called - Java must provide implementation at this point");
  }

  virtual double sin(double d) {
    throw std::logic_error("ECM::sin must never be called - Java must provide implementation at this point");
  }

  virtual double asin(double d) {
    throw std::logic_error("ECM::asin must never be called - Java must provide implementation at this point");
  }

  virtual double acos(double d) {
    throw std::logic_error("ECM::acos must never be called - Java must provide implementation at this point");
  }

  virtual double atan2(double d, double d1) {
    throw std::logic_error("ECM::atan2 must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalCylinder> newPhysicalCylinder() const {
    throw std::logic_error(
        "ECM::newPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> matrixRandomNoise3(double k) {
    throw std::logic_error(
        "ECM::matrixRandomNoise must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalSphere> newPhysicalSphere() {
    throw std::logic_error(
        "ECM::newPhysicalSphere must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<local_biology::NeuriteElement> newNeuriteElement() {
    throw std::logic_error(
        "ECM::newNeuriteElement must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<local_biology::SomaElement> newSomaElement() {
    throw std::logic_error("ECM::newSomaElement must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<cx3d::synapse::PhysicalSpine> newPhysicalSpine(
      const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length) {
    throw std::logic_error(
        "ECM::newPhysicalSpine must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<synapse::PhysicalBouton> newPhysicalBouton(const std::shared_ptr<physics::PhysicalObject>& po,
                                                                     const std::array<double, 2>& origin,
                                                                     double length) {
    throw std::logic_error(
        "ECM::newPhysicalBouton must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalBond> newPhysicalBond(const std::shared_ptr<physics::PhysicalObject>& a,
                                                                 const std::array<double, 2>& position_on_a,
                                                                 const std::shared_ptr<physics::PhysicalObject>& b,
                                                                 const std::array<double, 2>& position_on_b,
                                                                 double resting_length, double spring_constant) {
    throw std::logic_error(
        "ECM::newPhysicalBond must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalCylinder> getPhysicalCylinder(int i) {
    throw std::logic_error(
        "ECM::getPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<local_biology::NeuriteElement> getNeuriteElememt(int i) {
    throw std::logic_error(
        "ECM::getNeuriteElememt must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalNode> getPhysicalNode(int i) {
    throw std::logic_error(
        "ECM::getPhysicalNode must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<physics::PhysicalSphere> getPhysicalSphere(int i) {
    throw std::logic_error(
        "ECM::getPhysicalSphere must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<local_biology::SomaElement> getSomaElememt(int i) {
    throw std::logic_error("ECM::getSomaElememt must never be called - Java must provide implementation at this point");
  }
};

}  // namespace simulation
}  // namespace cx3d

#endif  // SIMULATION_ECM_H_
