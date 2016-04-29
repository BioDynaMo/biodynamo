#ifndef JAVA_UTIL_H_
#define JAVA_UTIL_H_

#include <vector>
#include <array>
#include <memory>
#include <algorithm>

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

using std::vector;
using spatial_organization::OpenTriangleOrganizer;

/**
 *  formerly used to delegate function calls to java - now deprecated - will be removed very soon
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

  static int randomHelper(int i);
};

/**
 * formerly used to delegate function calls to java - now deprecated - will be removed very soon
 */
template<class T>
class JavaUtil {
 public:
  virtual ~JavaUtil() {
  }

  /**
   * returns a
   */
  std::array<int, 4> generateTriangleOrder() {
    std::random_shuffle(triangle_order_.begin(), triangle_order_.end(), JavaUtil2::randomHelper);
    std::array<int, 4> ret;
    ret[0] = triangle_order_[0];
    ret[1] = triangle_order_[1];
    ret[2] = triangle_order_[2];
    ret[3] = triangle_order_[3];
    return ret;
  }

  /**
   * redirects call, because static methods cannot be handled by SWIG direcotr
   */
  std::shared_ptr<spatial_organization::OpenTriangleOrganizer<T>> oto_createSimpleOpenTriangleOrganizer() const {
    return OpenTriangleOrganizer<T>::createSimpleOpenTriangleOrganizer();
  }

 private:
  vector<int> triangle_order_ { 0, 1, 2, 3 };

};

}  // namespace cx3d

#endif  // JAVA_UTIL_H_
