#include "java_util.h"

#include <cmath>

#include "physics/physical_node.h"
#include "physics/physical_sphere.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"
#include "physics/physical_node_movement_listener.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "synapse/physical_spine.h"
#include "synapse/physical_bouton.h"

namespace cx3d {

using physics::PhysicalNode;
using physics::PhysicalSphere;
using physics::PhysicalCylinder;
using physics::PhysicalBond;
using physics::PhysicalNodeMovementListener;
using local_biology::SomaElement;
using local_biology::NeuriteElement;
using synapse::PhysicalSpine;
using synapse::PhysicalBouton;

JavaUtil2::JavaUtil2() {
}

void JavaUtil2::initPhysicalNodeMovementListener() const {
  PhysicalNodeMovementListener::setMovementOperationId((int) (10000 * getRandomDouble1()));
}

std::array<double, 3> JavaUtil2::matrixRandomNoise3(double k) const {
  std::array<double, 3> ret;
  ret[0] = -k + 2 * k * matrixNextRandomDouble();
  ret[1] = -k + 2 * k * matrixNextRandomDouble();
  ret[2] = -k + 2 * k * matrixNextRandomDouble();
  return ret;
}

Color JavaUtil2::getRandomColor() const {
  long r = std::lround(255 * getRandomDouble1());
  long g = std::lround(255 * getRandomDouble1());
  long b = std::lround(255 * getRandomDouble1());

  int color = 0xB3000000;
  color |= r << 16;
  color |= g << 8;
  color |= b;

  return Color(color);
}

double JavaUtil2::exp(double d) const {
  return std::exp(d);
}

double JavaUtil2::cbrt(double d) const {
  return std::cbrt(d);
}

double JavaUtil2::sqrt(double d) const {
  return std::sqrt(d);
}

double JavaUtil2::cos(double d) const {
  return std::cos(d);
}

double JavaUtil2::sin(double d) const {
  return std::sin(d);
}

double JavaUtil2::asin(double d) const {
  return std::asin(d);
}

double JavaUtil2::acos(double d) const {
  return std::acos(d);
}

double JavaUtil2::atan2(double d, double d1) const {
  return std::atan2(d, d1);
}

std::shared_ptr<physics::PhysicalCylinder> JavaUtil2::newPhysicalCylinder() const {
  return PhysicalCylinder::create();
}

std::shared_ptr<physics::PhysicalNode> JavaUtil2::newPhysicalNode() const {
  return PhysicalNode::create();
}

std::shared_ptr<physics::PhysicalNodeMovementListener> JavaUtil2::newPhysicalNodeMovementListener() const {
  return PhysicalNodeMovementListener::create();
}

std::shared_ptr<physics::PhysicalSphere> JavaUtil2::newPhysicalSphere() const {
  return PhysicalSphere::create();
}

std::shared_ptr<local_biology::NeuriteElement> JavaUtil2::newNeuriteElement() const {
  return NeuriteElement::create();
}

std::shared_ptr<local_biology::SomaElement> JavaUtil2::newSomaElement() const {
  return SomaElement::create();
}

std::shared_ptr<cx3d::synapse::PhysicalSpine> JavaUtil2::newPhysicalSpine(
    const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length) const {
  return PhysicalSpine::create(po, origin, length);
}

std::shared_ptr<synapse::PhysicalBouton> JavaUtil2::newPhysicalBouton(
    const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length) const {
  return PhysicalBouton::create(po, origin, length);
}

std::shared_ptr<physics::PhysicalBond> JavaUtil2::newPhysicalBond(const std::shared_ptr<physics::PhysicalObject>& a,
                                                                  const std::array<double, 2>& position_on_a,
                                                                  const std::shared_ptr<physics::PhysicalObject>& b,
                                                                  const std::array<double, 2>& position_on_b,
                                                                  double resting_length, double spring_constant) const {
  return PhysicalBond::create(a, position_on_a, b, position_on_b, resting_length, spring_constant);
}

}  // namespace cx3d
