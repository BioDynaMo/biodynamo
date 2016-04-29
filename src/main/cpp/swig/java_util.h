#ifndef JAVA_UTIL_H_
#define JAVA_UTIL_H_

#include <array>
#include <memory>

#include "color.h"
#include <spatial_organization/open_triangle_organizer.h>

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
template<class T> class SpatialOrganizationNodeMovementListener;
}  // namespace spatial_organization

namespace physics {

class Substance;
class IntracellularSubstance;
class PhysicalSphere;
class PhysicalCylinder;
class PhysicalObject;
class PhysicalBond;
class PhysicalNode;
class PhysicalNodeMovementListener;
//class ECMChemicalReaction;

}// namespace physics

/**
 * Contains functions to access former static methods that are still implemented in Java
 */
template<class T>
class JavaUtil {
 public:
  virtual ~JavaUtil() {
  }

  /**
   * returns a
   */
  virtual std::array<int, 4> generateTriangleOrder() {
    throw std::logic_error(
        "JavaUtil::generateTriangleOrder must never be called - Java must provide implementation at this point");
  }

  /**
   * redirects call, because static methods cannot be handled by SWIG direcotr
   */
  virtual std::shared_ptr<spatial_organization::OpenTriangleOrganizer<T>> oto_createSimpleOpenTriangleOrganizer() {
    throw std::logic_error(
        "JavaUtil::oto_createSimpleOpenTriangleOrganizer must never be called - Java must provide implementation at this point");
  }
};

/**
 *  java provided functions without templating
 */
class JavaUtil2 {
 public:
  virtual ~JavaUtil2() {
  }

  virtual void initPhysicalNodeMovementListener() const {
    throw std::logic_error(
        "JavaUtil2::initPhysicalNodeMovementListener must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> matrixRandomNoise3(double k) {
    throw std::logic_error(
        "JavaUtil2::matrixRandomNoise must never be called - Java must provide implementation at this point");
  }

  virtual double getRandomDouble1() {
    throw std::logic_error(
        "JavaUtil2::getRandomDouble must never be called - Java must provide implementation at this point");
  }

  virtual void setRandomSeed1(long seed) {
    throw std::logic_error(
        "JavaUtil2::setRandomSeed1 must never be called - Java must provide implementation at this point");
  }

  virtual double matrixNextRandomDouble() {
    throw std::logic_error(
        "JavaUtil2::matrixNextRandomDouble must never be called - Java must provide implementation at this point");
  }

  virtual double exp(double d) {
    throw std::logic_error("JavaUtil2::exp must never be called - Java must provide implementation at this point");
  }

  virtual double cbrt(double d) {
    throw std::logic_error("JavaUtil2::cbrt must never be called - Java must provide implementation at this point");
  }

  virtual double sqrt(double d) {
    throw std::logic_error("JavaUtil2::sqrt must never be called - Java must provide implementation at this point");
  }

  virtual double cos(double d) {
    throw std::logic_error("JavaUtil2::cos must never be called - Java must provide implementation at this point");
  }

  virtual double sin(double d) {
    throw std::logic_error("JavaUtil2::sin must never be called - Java must provide implementation at this point");
  }

  virtual double asin(double d) {
    throw std::logic_error("JavaUtil2::asin must never be called - Java must provide implementation at this point");
  }

  virtual double acos(double d) {
    throw std::logic_error("JavaUtil2::acos must never be called - Java must provide implementation at this point");
  }

  virtual double atan2(double d, double d1) {
    throw std::logic_error("JavaUtil2::atan2 must never be called - Java must provide implementation at this point");
  }

  std::shared_ptr<physics::PhysicalCylinder> newPhysicalCylinder() const;

  std::shared_ptr<physics::PhysicalNode> newPhysicalNode() const;

  std::shared_ptr<physics::PhysicalNodeMovementListener> newPhysicalNodeMovementListener() const;

  std::shared_ptr<physics::PhysicalSphere> newPhysicalSphere() const;

  std::shared_ptr<local_biology::NeuriteElement> newNeuriteElement() const;

  std::shared_ptr<local_biology::SomaElement> newSomaElement() const;

  std::shared_ptr<cx3d::synapse::PhysicalSpine> newPhysicalSpine(const std::shared_ptr<physics::PhysicalObject>& po,
                                                                 const std::array<double, 2>& origin,
                                                                 double length) const;

  std::shared_ptr<synapse::PhysicalBouton> newPhysicalBouton(const std::shared_ptr<physics::PhysicalObject>& po,
                                                             const std::array<double, 2>& origin, double length) const;

  std::shared_ptr<physics::PhysicalBond> newPhysicalBond(const std::shared_ptr<physics::PhysicalObject>& a,
                                                         const std::array<double, 2>& position_on_a,
                                                         const std::shared_ptr<physics::PhysicalObject>& b,
                                                         const std::array<double, 2>& position_on_b,
                                                         double resting_length, double spring_constant) const;

  virtual Color getRandomColor() {
    throw std::logic_error(
        "JavaUtil2::getRandomColor must never be called - Java must provide implementation at this point");
  }

  virtual double getGaussianDouble(double mean, double standard_deviation) {
    throw std::logic_error(
        "JavaUtil2::getGaussianDouble must never be called - Java must provide implementation at this point");
  }
};

}  // namespace cx3d

#endif  // JAVA_UTIL_H_
