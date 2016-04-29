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
  JavaUtil2();

  virtual ~JavaUtil2() {
  }

  // ---------------
  // Miscellaneous
  // ---------------

  void initPhysicalNodeMovementListener() const;

  std::array<double, 3> matrixRandomNoise3(double k) const;

  Color getRandomColor() const;

  // ---------------
  // Math functions
  // ---------------

  double exp(double d) const;

  double cbrt(double d) const;

  double sqrt(double d) const;

  double cos(double d) const;

  double sin(double d) const;

  double asin(double d) const;

  double acos(double d) const;

  double atan2(double d, double d1) const;

  // ---------------
  // Object creation
  // ---------------

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
  // ---------------
  // random numbers
  // ---------------

  double getRandomDouble1() const;

  void setRandomSeed1(long seed) const;

  double matrixNextRandomDouble() const;

  double getGaussianDouble(double mean, double standard_deviation) const;
};

}  // namespace cx3d

#endif  // JAVA_UTIL_H_
