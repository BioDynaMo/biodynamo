#include "physics/physical_bond.h"

#include <sstream>

#include "param.h"
#include "matrix.h"
#include "string_util.h"
#include "sim_state_serialization_util.h"
#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"

namespace cx3d {
namespace physics {

std::shared_ptr<PhysicalBond> PhysicalBond::create() {
  return std::shared_ptr<PhysicalBond>(new PhysicalBond());
}

std::shared_ptr<PhysicalBond> PhysicalBond::create(PhysicalObject* a, PhysicalObject* b) {
  auto pb = std::shared_ptr<PhysicalBond>(new PhysicalBond());
  pb->init(a, b);
  return pb;
}

std::shared_ptr<PhysicalBond> PhysicalBond::create(PhysicalObject* a,
                                                   const std::array<double, 2>& position_on_a,
                                                   PhysicalObject* b,
                                                   const std::array<double, 2>& position_on_b, double resting_length,
                                                   double spring_constant) {
  auto pb = std::shared_ptr<PhysicalBond>(new PhysicalBond());
  pb->init(a, position_on_a, b, position_on_b, resting_length, spring_constant);
  return pb;
}

PhysicalBond::PhysicalBond() {
}

void PhysicalBond::init(PhysicalObject* a, PhysicalObject* b) {
  dolocking(a, b);
  resting_length_ = Matrix::norm(Matrix::subtract(a_->getMassLocation(), b_->getMassLocation()));
  spring_constant_ = 10;
  dumping_constant_ = 0;
  sliding_allowed_ = false;
  past_length_ = resting_length_;
}

void PhysicalBond::init(PhysicalObject* a, const std::array<double, 2>& position_on_a, PhysicalObject* b,
                        const std::array<double, 2>& position_on_b,
                        double resting_length, double spring_constant) {
  dolocking(a, b);
  origin_on_a_ = position_on_a;
  origin_on_b_ = position_on_b;
  resting_length_ = resting_length;
  spring_constant_ = spring_constant;
  sliding_allowed_ = false;
  past_length_ = resting_length;
}

PhysicalObject* PhysicalBond::getFirstPhysicalObject() {
  return a_;
}

PhysicalObject* PhysicalBond::getSecondPhysicalObject() {
  return a_;
}

void PhysicalBond::setFirstPhysicalObject(PhysicalObject* a) {
  a_ = a;
}

void PhysicalBond::setSecondPhysicalObject(PhysicalObject* b) {
  b_ = b;
}

bool PhysicalBond::isHasEffectOnA() {
  return has_effect_on_a_;
}

void PhysicalBond::setHasEffectOnA(bool has_effect_on_a) {
  has_effect_on_a_ = has_effect_on_a;
}

bool PhysicalBond::isHasEffectOnB() {  //todo rename function
  return has_effect_on_b_;
}

void PhysicalBond::setHasEffectOnB(bool has_effect_on_b) {
  has_effect_on_b_ = has_effect_on_b;
}

bool PhysicalBond::isSlidingAllowed() {
  return sliding_allowed_;
}

void PhysicalBond::setSlidingAllowed(bool sliding_allowed) {
  sliding_allowed_ = sliding_allowed;
}

void PhysicalBond::exchangePhysicalObject(PhysicalObject* oldPo, PhysicalObject* newPo) {

  if (oldPo == a_) {
    a_ = newPo;
  } else {
    b_ = newPo;
  }
  oldPo->removePhysicalBond(this->shared_from_this());
  newPo->addPhysicalBond(this->shared_from_this());

}

void PhysicalBond::vanish() {
  a_->removePhysicalBond(this->shared_from_this());
  b_->removePhysicalBond(this->shared_from_this());
}

PhysicalObject* PhysicalBond::getOppositePhysicalObject(PhysicalObject* po) {
  if (po == a_) {
    return b_;
  } else {
    return a_;
  }
}

void PhysicalBond::setPositionOnObjectInLocalCoord(PhysicalObject* po,
                                                   const std::array<double, 2>& positionInLocalCoordinates) {
  if (po == a_) {
    origin_on_a_ = positionInLocalCoordinates;
  } else {
    origin_on_b_ = positionInLocalCoordinates;
  }
}

std::array<double, 2> PhysicalBond::getPositionOnObjectInLocalCoord(PhysicalObject* po) {
  if (po == a_) {
    return origin_on_a_;
  } else {
    return origin_on_b_;
  }
}

std::array<double, 4> PhysicalBond::getForceOn(PhysicalObject* po) {
  // 0. Find if this physicalBound has an effect at all on the object
  if ((po == a_ && !has_effect_on_a_) || (po == b_ && !has_effect_on_b_))
    return std::array<double, 4>( { 0.0, 0.0, 0.0, 0.0 });
  // 1. Find the other object
  auto other_po = getOppositePhysicalObject(po);
  // 2. Find the two insertion points of the bond
  auto point_on_other_po = other_po->transformCoordinatesPolarToGlobal(getPositionOnObjectInLocalCoord(other_po));
  auto point_on_po = po->transformCoordinatesPolarToGlobal(getPositionOnObjectInLocalCoord(po));
  // 3. Compute the force
  auto forceDirection = Matrix::subtract(point_on_other_po, point_on_po);
  // 3'. If sliding along the other object is possible,
  // only the component perpendicular to the xAxis of the other object is taken into account
  if (po == a_ && sliding_allowed_ && other_po->isAPhysicalCylinder()) {
    auto pc = static_cast<PhysicalCylinder*>(other_po);
    double proj_norm = Matrix::dot(forceDirection, other_po->getXAxis());
    auto new_position_on_other_po = getPositionOnObjectInLocalCoord(other_po);
    new_position_on_other_po[0] -= proj_norm;
    if (new_position_on_other_po[0] > pc->getActualLength() + 1) {
      auto d_l = pc->getDaughterLeft();
      if (d_l != nullptr) {
        exchangePhysicalObject(pc, d_l);
        new_position_on_other_po[0] = new_position_on_other_po[0] - pc->getActualLength();
      } else {
        new_position_on_other_po[0] += proj_norm;
      }
    } else if (new_position_on_other_po[0] < -1) {
      auto mo = pc->getMother();
      if (mo->isAPhysicalCylinder()) {
        auto m = static_cast<PhysicalCylinder*>(mo);
        exchangePhysicalObject(pc, m);
        new_position_on_other_po[0] = m->getActualLength() + new_position_on_other_po[0];
      } else {
        new_position_on_other_po[0] += proj_norm;
      }
    }
  }
  double actual_length = Matrix::norm(forceDirection);
  if (actual_length == 0) {  // should never be the case, but who knows... then we avoid division by 0
    return std::array<double, 4>( { 0.0, 0.0, 0.0, 0.0 });
  }

  double spring_speed = (actual_length - past_length_) / Param::kSimulationTimeStep;
  past_length_ = actual_length;

  double tension = spring_constant_ * (actual_length - resting_length_) + dumping_constant_ * spring_speed;  // (Note: different than in PhysicalCylinder)
  auto force = Matrix::scalarMult(tension / actual_length, forceDirection);

  // 4. Return it
  // TODO : cleaner way to to this...
  if (po->isAPhysicalCylinder()) {
    auto pc = static_cast<PhysicalCylinder*>(po);
//      return new double[] {force[0], force[1], force[2], 1-(getPositionOnObjectInLocalCoord(po)[0]/pc.getActualLength()) };
    double p = 1 - (getPositionOnObjectInLocalCoord(po)[0] / pc->getActualLength());
    if (p > 0.8) {
      p = 0.8;
    } else if (p < 0.2) {
      p = 0.2;
    }
    // Note : if p=0.5 all the times, there is some displacements that can't be justified
    // and if it is allowed to be 0 or 1, we get kinkings...
    return std::array<double, 4>( { force[0], force[1], force[2], p });
  }
  return std::array<double, 4>( { force[0], force[1], force[2], 0 });
}

std::array<double, 3> PhysicalBond::getFirstEndLocation() {
  return a_->transformCoordinatesPolarToGlobal(getPositionOnObjectInLocalCoord(a_));
}

std::array<double, 3> PhysicalBond::getSecondEndLocation() {
  return b_->transformCoordinatesPolarToGlobal(getPositionOnObjectInLocalCoord(b_));
}

double PhysicalBond::getRestingLength() {
  return resting_length_;
}

void PhysicalBond::setRestingLength(double resting_length) {
  resting_length_ = resting_length;
}

double PhysicalBond::getSpringConstant() {
  return spring_constant_;
}

void PhysicalBond::setSpringConstant(double spring_constant) {
  spring_constant_ = spring_constant;
}

double PhysicalBond::getMaxTension() {
  return max_tension_;
}

void PhysicalBond::setMaxTension(double max_tension) {
  max_tension_ = max_tension;
}

double PhysicalBond::getDumpingConstant() {
  return dumping_constant_;
}

void PhysicalBond::setDumpingConstant(double dumping_constant) {
  dumping_constant_ = dumping_constant;
}

std::string PhysicalBond::toString() const {
  return "PhysicalBond";      //"My name is Bond, PhysicalBond ("+hashCode()+")";
}

bool PhysicalBond::equalTo(const std::shared_ptr<PhysicalBond>& other) const {
  return this == other.get();
}

StringBuilder& PhysicalBond::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  //a, b circular reference
  SimStateSerializationUtil::keyValue(sb, "originOnA", origin_on_a_);
  SimStateSerializationUtil::keyValue(sb, "originOnB", origin_on_b_);
  SimStateSerializationUtil::keyValue(sb, "restingLength", resting_length_);
  SimStateSerializationUtil::keyValue(sb, "springConstant", spring_constant_);
  SimStateSerializationUtil::keyValue(sb, "maxTension", max_tension_);
  SimStateSerializationUtil::keyValue(sb, "dumpingConstant", dumping_constant_);
  SimStateSerializationUtil::keyValue(sb, "pastLenght", past_length_);
  SimStateSerializationUtil::keyValue(sb, "slidingAllowed", sliding_allowed_);
  SimStateSerializationUtil::keyValue(sb, "hasEffectOnA", has_effect_on_a_);
  SimStateSerializationUtil::keyValue(sb, "hasEffectOnB", has_effect_on_b_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

void PhysicalBond::dolocking(PhysicalObject* a, PhysicalObject* b) {
  a_ = a;
  b_ = b;
  a_->addPhysicalBond(this->shared_from_this());
  b_->addPhysicalBond(this->shared_from_this());
}

}  // namespace physics
}  // namespace cx3d
