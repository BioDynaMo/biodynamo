#ifndef PHYSICS_ECM_H_
#define PHYSICS_ECM_H_

#include <array>
#include <string>
#include <memory>
#include <exception>

namespace cx3d {
namespace physics {

class Substance;
class IntracellularSubstance;
class PhysicalSphere;
class PhysicalCylinder;

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
  virtual std::shared_ptr<Substance> substanceInstance(const std::string& id) {
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
  virtual std::shared_ptr<IntracellularSubstance> intracellularSubstanceInstance(const std::string& id) {
    throw std::logic_error(
        "ECM::intracellularSubstanceInstance must never be called - Java must provide implementation at this point");
  }

  virtual void addPhysicalCylinder(const std::shared_ptr<PhysicalCylinder>& cyl) {
    throw std::logic_error(
        "ECM::addPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual void addPhysicalSphere(const std::shared_ptr<PhysicalSphere>& cyl) {
    throw std::logic_error(
        "ECM::addPhysicalSphere must never be called - Java must provide implementation at this point");
  }

  virtual bool getArtificialWallForSpheres() {
    throw std::logic_error(
        "ECM::getArtificialWallForSpheres must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> forceFromArtificialWall(const std::array<double, 3>& location, double radius) {
    throw std::logic_error(
        "ECM::forceFromArtificialWall must never be called - Java must provide implementation at this point");
  }

  virtual double getRandomDouble1() {
    throw std::logic_error(
        "ECM::getRandomDouble must never be called - Java must provide implementation at this point");
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

  virtual double atan2(double d, double d1) {
    throw std::logic_error("ECM::atan2 must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalCylinder> newPhysicalCylinder() const {
    throw std::logic_error(
        "ECM::newPhysicalCylinder must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> matrixRandomNoise3(double k) {
    throw std::logic_error(
        "ECM::matrixRandomNoise must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<PhysicalSphere> newPhysicalSphere() {
    throw std::logic_error(
        "ECM::newPhysicalSphere must never be called - Java must provide implementation at this point");
  }
};

}  // namespace physics
}  // namespace cx3d

#endif  // PHYSICS_ECM_H_
