#include "physics/physical_object.h"

#include <cmath>

#include "param.h"
#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "physics/physical_bond.h"
#include "physics/physical_sphere.h"
#include "physics/physical_cylinder.h"
#include "physics/inter_object_force.h"
#include "physics/intracellular_substance.h"
#include "spatial_organization/space_node.h"
#include "synapse/excrescence.h"

namespace cx3d {
namespace physics {

InterObjectForce::UPtr PhysicalObject::inter_object_force_ { nullptr };

InterObjectForce* PhysicalObject::getInterObjectForce() {
  return inter_object_force_.get();
}

void PhysicalObject::setInterObjectForce(InterObjectForce::UPtr force) {
  inter_object_force_ = std::move(force);
}

PhysicalObject::PhysicalObject() {
}

PhysicalObject::~PhysicalObject() {
}

StringBuilder& PhysicalObject::simStateToJson(StringBuilder& sb) const {
  PhysicalNode::simStateToJson(sb);
  SimStateSerializationUtil::removeLastChar(sb);
  sb.append(",");

  SimStateSerializationUtil::keyValue(sb, "interObjectForce", inter_object_force_.get());
  SimStateSerializationUtil::keyValue(sb, "stillExisting", still_existing_);
  SimStateSerializationUtil::keyValue(sb, "onTheSchedulerListForPhysicalObjects",
                                      on_scheduler_list_for_physical_objects_);
  SimStateSerializationUtil::keyValue(sb, "massLocation", mass_location_);
  SimStateSerializationUtil::keyValue(sb, "xAxis", x_axis_);
  SimStateSerializationUtil::keyValue(sb, "yAxis", y_axis_);
  SimStateSerializationUtil::keyValue(sb, "zAxis", z_axis_);
  SimStateSerializationUtil::keyValue(sb, "adherence", adherence_);
  SimStateSerializationUtil::keyValue(sb, "mass", mass_);
  SimStateSerializationUtil::keyValue(sb, "diameter", diameter_);
  SimStateSerializationUtil::keyValue(sb, "volume", volume_);
  SimStateSerializationUtil::keyValue(sb, "color", SimStateSerializationUtil::colorToHexString(color_.getValue()),
                                      true);
  SimStateSerializationUtil::keyValue(sb, "totalForceLastTimeStep", total_force_last_time_step_);
  SimStateSerializationUtil::map(sb, "intracellularSubstances", intracellular_substances_);
  SimStateSerializationUtil::unorderedCollection(sb, "physicalBonds", physical_bonds_);
  SimStateSerializationUtil::unorderedCollection(sb, "excrescences", excrescences_);

  return sb;
}

bool PhysicalObject::isAPhysicalObject() const {
  return true;
}

void PhysicalObject::addExcrescence(Excrescence::UPtr ex) {
  excrescences_.push_back(std::move(ex));
}

void PhysicalObject::removeExcrescence(Excrescence* ex) {
  STLUtil::vectorRemove(excrescences_, ex);
}

bool PhysicalObject::isInContact(PhysicalObject* o) {
  if (o->isAPhysicalSphere()) {
    return isInContactWithSphere(static_cast<PhysicalSphere*>(o));
  } else {
    return isInContactWithCylinder(static_cast<PhysicalCylinder*>(o));
  }
}

std::list<PhysicalObject*> PhysicalObject::getPhysicalObjectsInContact() {
  std::list<PhysicalObject*> po;
  for (auto n : getSoNode()->getNeighbors()) {
    if (n->isAPhysicalObject() && isInContact(static_cast<PhysicalObject*>(n))) {
      po.push_back(static_cast<PhysicalObject*>(n));
    }
  }
  return po;
}

std::array<double, 3> PhysicalObject::transformCoordinatesGlobalToLocal(
    const std::array<double, 3>& positionInGlobalCoord) const {
  return std::array<double, 3> { Matrix::dot(positionInGlobalCoord, x_axis_), Matrix::dot(positionInGlobalCoord,
                                                                                          y_axis_), Matrix::dot(
      positionInGlobalCoord, z_axis_) };
}

std::array<double, 3> PhysicalObject::transformCoordinatesLocalToGlobal(
    const std::array<double, 3>& positionInLocalCoord) const {
  return std::array<double, 3> { positionInLocalCoord[0] * x_axis_[0] + positionInLocalCoord[1] * y_axis_[0]
      + positionInLocalCoord[2] * z_axis_[0], positionInLocalCoord[0] * x_axis_[1]
      + positionInLocalCoord[1] * y_axis_[1] + positionInLocalCoord[2] * z_axis_[1], positionInLocalCoord[0]
      * x_axis_[2] + positionInLocalCoord[1] * y_axis_[2] + positionInLocalCoord[2] * z_axis_[2] };
}

void PhysicalObject::addPhysicalBond(const std::shared_ptr<PhysicalBond>& bond) {
  physical_bonds_.push_back(bond);
}

void PhysicalObject::removePhysicalBond(const std::shared_ptr<PhysicalBond>& bond) {
  physical_bonds_.remove(bond);
}

bool PhysicalObject::getHasAPhysicalBondWith(PhysicalObject* po) {
  for (auto pb : physical_bonds_) {
    if (po == pb->getOppositePhysicalObject(this)) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<PhysicalBond> PhysicalObject::makePhysicalBondWith(PhysicalObject* po) {
  return PhysicalBond::create(this, po);
}

bool PhysicalObject::removePhysicalBondWith(PhysicalObject* po, bool removeThemAll) {
  bool there_was_a_bond = false;
  std::size_t i = 0;
  auto it = physical_bonds_.begin();
  while (it != physical_bonds_.end()) {
    auto pb = *it;
    if (po == pb->getOppositePhysicalObject(this)) {
      physical_bonds_.remove(*it);
      po->physical_bonds_.remove(pb);
      if (!removeThemAll) {
        return true;
      } else {
        there_was_a_bond = true;
        i--;  // we continue to check, and since we removed the ith
      }
    }
    i++;
    it++;
  }
  return there_was_a_bond;
}

double PhysicalObject::getIntracellularConcentration(const std::string& substanceId) {
  if (!STLUtil::mapContains(intracellular_substances_, substanceId)) {
    return 0;
  } else {
    return intracellular_substances_[substanceId]->getConcentration();
  }
}

void PhysicalObject::modifyIntracellularQuantity(const std::string& id, double quantityPerTime) {

  IntracellularSubstance* s = nullptr;
  if (!STLUtil::mapContains(intracellular_substances_, id)) {
    auto s_uptr = PhysicalNode::ecm_->intracellularSubstanceInstance(id);
    s = s_uptr.get();
    intracellular_substances_[id] = std::move(s_uptr);
  } else {
    s = intracellular_substances_[id].get();
  }
  double delta_q = quantityPerTime * Param::kSimulationTimeStep;
  s->changeQuantityFrom(delta_q);
  if (s->isVolumeDependant()) {
    s->updateConcentrationBasedOnQuantity(volume_);
  } else {
    s->updateConcentrationBasedOnQuantity(getLength());
  }
}

double PhysicalObject::getMembraneConcentration(const std::string& id) {
  if (id == "U") {
    return 1.0;
  }
  // otherwise : do we have it on board ?
  if (!STLUtil::mapContains(intracellular_substances_, id)) {
    return 0.0;
  } else {
    // if yes, is it a membrane substance ?
    auto s = intracellular_substances_[id].get();
    if (!s->isVisibleFromOutside()) {
      return 0.0;
    }
    return s->getConcentration();
  }
}

void PhysicalObject::modifyMembraneQuantity(const std::string& id, double quantityPerTime) {
  // for now, the intracellular and membrane bound Substances are the same.
  modifyIntracellularQuantity(id, quantityPerTime);
}

IntracellularSubstance* PhysicalObject::giveYouIntracellularSubstanceInstance(IntracellularSubstance* templateS) {
  IntracellularSubstance* s = nullptr;
  if (!STLUtil::mapContains(intracellular_substances_, templateS->getId())) {
    auto s_uptr = IntracellularSubstance::UPtr(new IntracellularSubstance(*templateS));
    s = s_uptr.get();
    intracellular_substances_[s->getId()] = std::move(s_uptr);
  } else {
    s = intracellular_substances_[templateS->getId()].get();
  }
  return s;
}

void PhysicalObject::diffuseWithThisPhysicalObjects(PhysicalObject* po, double distance) {
  // We store these temporary variable, because we still don't know if the
  // Substances depend on volumes or not

  double vA_v = volume_;
  double vB_v = po->getVolume();
  double pre_a_v = (1.0 / distance);
  double pre_m_v = (1.0 / distance) * (1.0 / vA_v + 1.0 / vB_v);
  double vA_l = getLength();
  double vB_l = po->getLength();
  double pre_a_l = (1.0 / distance);
  double pre_m_l = (1.0 / distance) * (1.0 / vA_l + 1.0 / vB_l);

  // the variable we are effectively using
  double vA;
  double vB;
  double pre_a;
  double pre_m;

  for (auto& element : intracellular_substances_) {
    // for a given substance
    auto s_a = element.second.get();

    // does the substance depend on volumes or not ?
    if (s_a->isVolumeDependant()) {
      vA = vA_v;
      vB = vB_v;
      pre_a = pre_a_v;
      pre_m = pre_m_v;
    } else {
      vA = vA_l;
      vB = vB_l;
      pre_a = pre_a_l;
      pre_m = pre_m_l;
    }

    double s_a_concentration = s_a->getConcentration();

    // stop here if 1) non diffusible substance or 2) concentration very low:
    double diffusion_constant = s_a->getDiffusionConstant();
    if (diffusion_constant < 10E-14 || s_a_concentration < Param::kMinimalConcentrationForIntracellularDiffusion) {
      continue;  // to avoid a division by zero in the n/m if the diff const = 0;
    }

    // find the counterpart in po
    auto s_b = po->giveYouIntracellularSubstanceInstance(s_a);
    double s_b_concentration = s_b->getConcentration();

    // saving time : no diffusion if almost no difference;
    double abs_diff = std::abs(s_a_concentration - s_b_concentration);
    if ((abs_diff < Param::kMinimalConcentrationForIntracellularDiffusion)
        || (abs_diff / s_a_concentration < Param::kMinimalDCOverCForIntracellularDiffusion)) {
      continue;
    }
    // TODO : if needed, when we come here, we have to re-put ourselves on the
    // scheduler list for intra-cellular diffusion.

    // analytic computation of the diffusion between these two PhysicalObjects
    // (cf document "Diffusion" by F.Zubler for explanation).
    double q_a = s_a->getQuantity();
    double q_b = s_b->getQuantity();
    double Tot = q_a + q_b;
    double a = pre_a * diffusion_constant;
    double m = pre_m * diffusion_constant;
    double n = a * Tot / vB;
    double n_over_m = n / m;
    double K = q_a - n_over_m;
    q_a = K * MathUtil::exp(-m * Param::kSimulationTimeStep) + n_over_m;
    q_b = Tot - q_a;

    s_a->setQuantity(q_a);
    s_b->setQuantity(q_b);
    // and update their concentration again
    // (Recall : if volumeDependant = false, vA and vB are not the true volume,
    // but rather the length of the physical object)
    s_a->updateConcentrationBasedOnQuantity(vA);
    s_b->updateConcentrationBasedOnQuantity(vB);
  }
}

Color PhysicalObject::getColor() const {
  return color_;
}

void PhysicalObject::setColor(Color c) {
  color_ = c;
}

std::array<double, 3> PhysicalObject::getMassLocation() const {
  return std::array<double, 3> { mass_location_[0], mass_location_[1], mass_location_[2] };  //todo copy really needed?
}

void PhysicalObject::setMassLocation(const std::array<double, 3>& mass_location) {
  mass_location_ = mass_location;
}

std::array<double, 3> PhysicalObject::getXAxis() const {
  return x_axis_;
}

void PhysicalObject::setXAxis(const std::array<double, 3>& axis) {
  x_axis_ = axis;
}

std::array<double, 3> PhysicalObject::getYAxis() const {
  return y_axis_;
}

void PhysicalObject::setYAxis(const std::array<double, 3>& axis) {
  y_axis_ = axis;
}

std::array<double, 3> PhysicalObject::getZAxis() const {
  return z_axis_;
}

void PhysicalObject::setZAxis(const std::array<double, 3>& axis) {
  z_axis_ = axis;
}

std::array<double, 4> PhysicalObject::getTotalForceLastTimeStep() const {
  return total_force_last_time_step_;
}

bool PhysicalObject::isStillExisting() const {
  return still_existing_;
}

void PhysicalObject::setStillExisting(bool stillExists) {
  still_existing_ = stillExists;
}

bool PhysicalObject::isOnTheSchedulerListForPhysicalObjects() const {
  return on_scheduler_list_for_physical_objects_;
}

void PhysicalObject::setOnTheSchedulerListForPhysicalObjects(bool on_scheduler_list) {
  on_scheduler_list_for_physical_objects_ = on_scheduler_list;
}

std::list<std::shared_ptr<PhysicalBond>> PhysicalObject::getPhysicalBonds() const {
  std::list < std::shared_ptr<PhysicalBond> > list;
  for (auto pb : physical_bonds_) {
    list.push_back(pb);
  }
  return list;
}

void PhysicalObject::setPhysicalBonds(const std::list<std::shared_ptr<PhysicalBond> >& bonds) {  //todo change to vector
  physical_bonds_ = bonds;
}

std::vector<Excrescence*> PhysicalObject::getExcrescences() const {
  std::vector<Excrescence*> ret;
  for (auto& ex : excrescences_) {
    ret.push_back(ex.get());
  }
  return ret;
}

double PhysicalObject::getAdherence() const {
  return adherence_;
}

void PhysicalObject::setAdherence(double a) {
  adherence_ = a;
}

double PhysicalObject::getMass() const {
  return mass_;
}

void PhysicalObject::setMass(double m) {
  mass_ = m;
}

double PhysicalObject::getDiameter() const {
  return diameter_;
}

void PhysicalObject::setDiameter(double diameter) {
  setDiameter(diameter, true);
}

void PhysicalObject::setDiameter(double d, bool update_volume) {
  diameter_ = d;
  if (update_volume) {
    updateVolume();
  }
}

double PhysicalObject::getVolume() const {
  return volume_;
}

void PhysicalObject::setVolume(double v, bool update_diameter) {
  volume_ = v;
  updateIntracellularConcentrations();
  if (update_diameter) {
    updateDiameter();
  }
}

void PhysicalObject::setVolume(double volume) {
  setVolume(volume, true);
}

IntracellularSubstance* PhysicalObject::getIntracellularSubstance(const std::string& id) {
  if (STLUtil::mapContains(intracellular_substances_, id)) {
    return intracellular_substances_[id].get();
  }
  return nullptr;
}

void PhysicalObject::addIntracellularSubstance(IntracellularSubstance::UPtr is) {
  intracellular_substances_[is->getId()] = std::move(is);
}

void PhysicalObject::removeIntracellularSubstance(IntracellularSubstance* is) {
  intracellular_substances_.erase(is->getId());
}

std::list<IntracellularSubstance*> PhysicalObject::getIntracellularSubstances1() const {
  std::list<IntracellularSubstance*> list;
  for (auto& el : intracellular_substances_) {
    list.push_back(el.second.get());
  }
  return list;
}

void PhysicalObject::addNewIntracellularSubstance(IntracellularSubstance::UPtr s) {
  intracellular_substances_[s->getId()] = std::move(s);
}

void PhysicalObject::setVolumeOnly(double v) {
  volume_ = v;
}

}  // namespace physics
}  // namespace cx3d
