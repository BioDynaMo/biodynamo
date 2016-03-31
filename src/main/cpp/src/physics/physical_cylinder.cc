#include "physics/physical_cylinder.h"

#include <exception>

#include "matrix.h"
#include "string_util.h"
#include "sim_state_serialization_util.h"

#include "local_biology/cell_element.h"
#include "local_biology/neurite_element.h"
#include "local_biology/local_biology_module.h"

#include "physics/physical_bond.h"
#include "physics/physical_sphere.h"
#include "physics/intracellular_substance.h"
#include "physics/inter_object_force.h"
#include "physics/debug/physical_cylinder_debug.h"

#include "synapse/excrescence.h"

#include "spatial_organization/space_node.h"

namespace cx3d {
namespace physics {

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::create() {
  return std::shared_ptr<PhysicalCylinder>(new PhysicalCylinder());
//  return std::shared_ptr<PhysicalCylinder>(new PhysicalCylinderDebug());
}

PhysicalCylinder::PhysicalCylinder() {
  adherence_ = Param::kNeuriteDefaultAdherence;
  mass_ = Param::kNeuriteDefaultMass;
  diameter_ = Param::kNeuriteDefaultDiameter;
  updateVolume();
}

StringBuilder& PhysicalCylinder::simStateToJson(StringBuilder& sb) const {
  PhysicalObject::simStateToJson(sb);

  //motherNode, neuriteElementm daughterLeft, daughterRight are circular references
  SimStateSerializationUtil::keyValue(sb, "branchOrder", branch_order_);
  SimStateSerializationUtil::keyValue(sb, "forceToTransmitToProximalMass", force_to_transmit_to_proximal_mass_);
  SimStateSerializationUtil::keyValue(sb, "springAxis", spring_axis_);
  SimStateSerializationUtil::keyValue(sb, "actualLength", actual_length_);
  SimStateSerializationUtil::keyValue(sb, "tension", tension_);
  SimStateSerializationUtil::keyValue(sb, "springConstant", spring_constant_);
  SimStateSerializationUtil::keyValue(sb, "restingLength", resting_length_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

std::string PhysicalCylinder::toString() const {
  std::ostringstream str_stream;
  str_stream << "ini.cx3d.physics.PhyicalCylinder" << getID();
  return str_stream.str();
//  std::ostringstream str_stream;
//  str_stream << "(";
//  str_stream << StringUtil::toStr(physical_bonds_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(excrescences_.size());
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(neurite_element_);
//  str_stream << ", ";
//  str_stream << (mother_ != nullptr ? StringUtil::toStr(mother_->getID()) : "null");
//  str_stream << ", ";
//  str_stream << (daughter_left_ != nullptr ? StringUtil::toStr(daughter_left_->getID()) : "null");
//  str_stream << ", ";
//  str_stream << (daughter_right_ != nullptr ? StringUtil::toStr(daughter_right_->getID()) : "null");
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(branch_order_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(force_to_transmit_to_proximal_mass_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(spring_axis_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(actual_length_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(tension_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(spring_constant_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(resting_length_);
//  str_stream << ")";
//  return str_stream.str();
}

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::getCopy() const {

  auto new_cylinder = ecm_->newPhysicalCylinder();

  // PhysicalObject variables
  new_cylinder->adherence_ = adherence_;
  new_cylinder->mass_ = mass_;
  new_cylinder->setDiameter(diameter_, true);  // re - computes also volumes
  new_cylinder->color_ = color_;
  new_cylinder->still_existing_ = still_existing_;

  new_cylinder->x_axis_ = x_axis_;
  new_cylinder->y_axis_ = y_axis_;
  new_cylinder->z_axis_ = z_axis_;
  // this class variable
  new_cylinder->spring_axis_ = spring_axis_;
  new_cylinder->branch_order_ = branch_order_;
  new_cylinder->spring_constant_ = spring_constant_;
  return new_cylinder;
}

bool PhysicalCylinder::isRelative(const std::shared_ptr<PhysicalObject>& po) const {
  // mother-daughter
  if (po == mother_ || po == daughter_left_ || po == daughter_left_) {
    return true;
  }
  // sister-sister
  if (po->isAPhysicalCylinder() && std::static_pointer_cast<PhysicalCylinder>(po) == mother_) {
    return true;
  }
  return false;
}

std::array<double, 3> PhysicalCylinder::originOf(const std::shared_ptr<PhysicalObject>& daughter) {
  // TODO : consider remove the check
  if (daughter == daughter_left_ || daughter == daughter_right_) {
    return mass_location_;
  }
  throw std::logic_error("PhysicalCylinder.getOrigin() says : this is not one of my relatives !!!");
}

void PhysicalCylinder::removeDaugther(const std::shared_ptr<PhysicalObject>& daughter) {
  // If there is another daughter than the one we want to remove,
  // we have to be sure that it will be the daughterLeft->
  if (daughter == daughter_right_) {
    daughter_right_ = std::shared_ptr<PhysicalCylinder> { nullptr };
    return;
  }

  if (daughter == daughter_left_) {
    daughter_left_ = daughter_right_;
    daughter_right_ = std::shared_ptr<PhysicalCylinder> { nullptr };
    return;
  }
  throw std::logic_error("PhysicalCylinder.removeDaugther() says : this is not one of my relatives !!!");
}

void PhysicalCylinder::updateRelative(const std::shared_ptr<PhysicalObject>& old_relative,
                                      const std::shared_ptr<PhysicalObject>& new_relative) {
  if (old_relative == mother_) {
    setMother(new_relative);
    return;
  }
  if (old_relative == daughter_left_) {
    setDaughterLeft(std::static_pointer_cast<PhysicalCylinder>(new_relative));
    return;
  }
  if (old_relative == daughter_right_) {
    setDaughterRight(std::static_pointer_cast<PhysicalCylinder>(new_relative));
    return;
  }
  throw std::logic_error("PhysicalCylinder.updateRelative() says : this is not one of my relatives !!!");
}

std::array<double, 3> PhysicalCylinder::forceTransmittedFromDaugtherToMother(const std::shared_ptr<PhysicalObject>& m) {
  if (m != mother_) {
    throw std::logic_error(
        "PhysicalCylinder.forceTransmittedFromDaugtherToMother() says : this is not one of my relatives !!!");
  }
  // The inner tension is Matrix::added to the external force that was computed earlier.
  // (The reason for dividing by the actualLength is to Matrix::normalize the direction : T = T * axis/ (axis length)
  double factor = tension_ / actual_length_;
  if (factor < 0) {
    factor = 0;
  }
  return {
    factor*spring_axis_[0] + force_to_transmit_to_proximal_mass_[0],
    factor*spring_axis_[1] + force_to_transmit_to_proximal_mass_[1],
    factor*spring_axis_[2] + force_to_transmit_to_proximal_mass_[2]
  };
}

bool PhysicalCylinder::runDiscretization() {

  if (actual_length_ > Param::kNeuriteMaxLength) {
    if (daughter_left_ == nullptr) {   // if terminal branch :
      insertProximalCylinder(0.1);
    } else if (mother_->isAPhysicalSphere()) {  //if initial branch :
      insertProximalCylinder(0.9);
    } else {
      insertProximalCylinder(0.5);
    }
    return true;
  }

  if (actual_length_ < Param::kNeuriteMinLength && mother_->isAPhysicalCylinder()
      && std::static_pointer_cast<PhysicalCylinder>(mother_)->resting_length_
          < Param::kNeuriteMaxLength - resting_length_ - 1
      && std::static_pointer_cast<PhysicalCylinder>(mother_)->daughter_right_ == nullptr && daughter_left_ != nullptr) {
    // if the previous branch is removed, we first remove its associated NeuriteElement
    std::static_pointer_cast<PhysicalCylinder>(mother_)->neurite_element_->removeYourself();
    // then we remove it
    removeProximalCylinder();
  }
  return true;
}

void PhysicalCylinder::extendCylinder(double speed, const std::array<double, 3>& direction) {
  double temp = Matrix::dot(direction, spring_axis_);
  if (temp > 0) {
    movePointMass(speed, direction);
  }
}

void PhysicalCylinder::movePointMass(double speed, const std::array<double, 3>& direction) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return;
  }

  // scaling for integration step
  double length = speed * Param::kSimulationTimeStep;
  auto normalized_dir = Matrix::normalize(direction);
  std::array<double, 3> displacement { length * normalized_dir[0], length * normalized_dir[1], length
      * normalized_dir[2] };
  mass_location_ = Matrix::add(displacement, mass_location_);
  // here I have to define the actual length ..........
  auto relative_ml = mother_->originOf(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));  // mass_location_ of the mother
  spring_axis_[0] = mass_location_[0] - relative_ml[0];
  spring_axis_[1] = mass_location_[1] - relative_ml[1];
  spring_axis_[2] = mass_location_[2] - relative_ml[2];
  actual_length_ = ecm_->sqrt(
      spring_axis_[0] * spring_axis_[0] + spring_axis_[1] * spring_axis_[1] + spring_axis_[2] * spring_axis_[2]);
  // process of elongation : setting tension to 0 increases the restingLength :
  setRestingLengthForDesiredTension(0.0);

  // some physics and computation obligations....
  updateVolume();  // and update concentration of internal stuff.
  updateLocalCoordinateAxis();
  updateSpatialOrganizationNodePosition();
  // Make sure I'll be updated when I run my physics
  // but since I am actually moving, I have to update the neighbors
  // (the relative would probably not be needed...).
  scheduleMeAndAllMyFriends();
}

bool PhysicalCylinder::retractCylinder(double speed) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return true;
  }
  // TODO : what if there are some physical Bonds ??
  // scaling for integration step
  speed = speed * Param::kSimulationTimeStep;

  // if aL > length : retraction keeping the same tension
  // (putting a limit on how short a branch can be is absolutely necessary
  //   otherwise the tension might explode)
  if (actual_length_ > speed + 0.1) {
    double new_actual_length = actual_length_ - speed;
    double factor = new_actual_length / actual_length_;
    actual_length_ = new_actual_length;
    resting_length_ = spring_constant_ * actual_length_ / (tension_ + spring_constant_);  //cf removeproximalCylinder()
    spring_axis_ = {factor*spring_axis_[0], factor*spring_axis_[1], factor*spring_axis_[2]};
    mass_location_ = Matrix::add(mother_->originOf(std::static_pointer_cast<PhysicalObject>(this->shared_from_this())),
                                 spring_axis_);
    updateVolume();  // and update concentration of internal stuff.
    updateSpatialOrganizationNodePosition();
    // be sure i'll run my physics :
    setOnTheSchedulerListForPhysicalObjects(true);
    return true;
    // if al < length and mother is a PhysicalCylinder with no other daughter : merge with mother
  } else if (mother_->isAPhysicalCylinder()
      && std::static_pointer_cast<PhysicalCylinder>(mother_)->getDaughterRight() == nullptr) {
    removeProximalCylinder();  // also updates volume_...
    // be sure i'll run my physics :
    setOnTheSchedulerListForPhysicalObjects(true);
    return retractCylinder(speed / Param::kSimulationTimeStep);
    // if mother is cylinder with other daughter or is not a cylinder : disappear.
  } else {
    mother_->removeDaugther(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
    still_existing_ = false;
    ecm_->removePhysicalCylinder(std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));  // this method removes the SONode
    // and the associated neuriteElement also disappears :
    neurite_element_->removeYourself();
    // intracellularSubstances quantities
    // (concentrations are solved in updateDependentPhysicalVariables():
    for (auto el : intracellular_substances_) {
      auto s = el.second;
      mother_->modifyIntracellularQuantity(s->getId(), s->getQuantity() / Param::kSimulationTimeStep);
      // (divide by time step because it is multiplied by it in the method)
    }
    mother_->updateDependentPhysicalVariables();
    // extra-safe : make sure you'll not be run :
    setOnTheSchedulerListForPhysicalObjects(false);
    return false;
  }
}

std::array<std::shared_ptr<PhysicalCylinder>, 2> PhysicalCylinder::bifurcateCylinder(
    double length, const std::array<double, 3>& direction_1, const std::array<double, 3>& direction_2) {
  // check it is a terminal branch
  if (daughter_left_ != nullptr) {
    throw std::logic_error("Not a terminal Branch");  //todo should return nullptr
  }
  // create the cylinders
  auto new_branch_l = getCopy();
  auto new_branch_r = getCopy();
  // set family relations
  daughter_left_ = new_branch_l;
  new_branch_l->setMother(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
  daughter_right_ = new_branch_r;
  new_branch_r->setMother(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));

  // check that the directions are not pointing backwards
  auto dir_1 = direction_1;  // todo avoid cpy
  auto dir_2 = direction_2;
  if (Matrix::angleRadian(spring_axis_, direction_1) > Param::kPi * 0.5) {
    auto proj = Matrix::projectionOnto(direction_1, spring_axis_);
    proj = Matrix::scalarMult(-1, proj);
    dir_1 = Matrix::add(direction_1, proj);
  }
  if (Matrix::angleRadian(spring_axis_, direction_2) > Param::kPi * 0.5) {
    auto proj = Matrix::projectionOnto(direction_2, spring_axis_);
    proj = Matrix::scalarMult(-1, proj);
    dir_2 = Matrix::add(direction_2, proj);
  }

  // mass location and spring axis
  new_branch_l->spring_axis_ = Matrix::scalarMult(length, Matrix::normalize(dir_1));
  new_branch_l->mass_location_ = Matrix::add(mass_location_, new_branch_l->spring_axis_);
  new_branch_l->updateLocalCoordinateAxis();  // (important so that x_axis_ is correct)

  new_branch_r->spring_axis_ = Matrix::scalarMult(length, Matrix::normalize(dir_2));
  new_branch_r->mass_location_ = Matrix::add(mass_location_, new_branch_r->spring_axis_);
  new_branch_r->updateLocalCoordinateAxis();

  // physics of tension :
  new_branch_l->setActualLength(length);
  new_branch_r->setActualLength(length);
  new_branch_r->setRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  new_branch_l->setRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);

  // spatial organization node
  auto new_branch_center = Matrix::add(mass_location_, Matrix::scalarMult(0.5, new_branch_l->spring_axis_));
  auto new_son = so_node_->getNewInstance(new_branch_center, new_branch_l);  // todo catch PositionNotAllowedException
  new_branch_l->setSoNode(new_son);

  new_branch_center = Matrix::add(mass_location_, Matrix::scalarMult(0.5, new_branch_r->spring_axis_));
  new_son = so_node_->getNewInstance(new_branch_center, new_branch_r);  // todo catch PositionNotAllowedException
  new_branch_r->setSoNode(new_son);

  // register the new branches with ecm
  ecm_->addPhysicalCylinder(new_branch_l);
  ecm_->addPhysicalCylinder(new_branch_r);

  // set local coordinate axis in the new branches
  new_branch_l->updateLocalCoordinateAxis();
  new_branch_r->updateLocalCoordinateAxis();

  // i'm scheduled to run physics next time :
  // (the daughters automatically are too, because they are new PhysicalObjects)
  setOnTheSchedulerListForPhysicalObjects(true);

  new_branch_l->updateDependentPhysicalVariables();
  new_branch_r->updateDependentPhysicalVariables();

  return {daughter_left_, daughter_right_};
}

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::branchCylinder(double length,
                                                                   const std::array<double, 3>& direction) {
  // we first split this cylinder into two pieces
  auto ne = insertProximalCylinder();
  // then append a "daughter right" between the two
  return ne->getPhysicalCylinder()->extendSideCylinder(length, direction);
}

void PhysicalCylinder::setRestingLengthForDesiredTension(double tension) {
  tension_ = tension;
  // T = k*(A-R)/R --> R = k*A/(T+K)
  resting_length_ = spring_constant_ * actual_length_ / (tension_ + spring_constant_);
}

void PhysicalCylinder::changeVolume(double speed) {
  //scaling for integration step
  double dV = speed * (Param::kSimulationTimeStep);
  volume_ += dV;

  if (volume_ < 5.2359877E-7) {  // minimum volume_, corresponds to minimal diameter_
    volume_ = 5.2359877E-7;
  }
  updateDiameter();
  updateIntracellularConcentrations();
  scheduleMeAndAllMyFriends();
}

void PhysicalCylinder::changeDiameter(double speed) {
  //scaling for integration step
  double dD = speed * (Param::kSimulationTimeStep);
  diameter_ += dD;
  updateVolume();
  // no call to updateIntracellularConcentrations() cause it's done by updateVolume().
  scheduleMeAndAllMyFriends();
}

void PhysicalCylinder::runPhysics() {
  // decide first if we have to split or fuse this cylinder. Usually only
  // terminal branches (growth cone) do
  if (daughter_left_ == nullptr) {
    runDiscretization();
  }

  // in case we don't move, we won't run physics the next time :
  setOnTheSchedulerListForPhysicalObjects(false);

  double h = Param::kSimulationTimeStep;
  std::array<double, 3> force_on_my_point_mass { 0, 0, 0 };
  std::array<double, 3> force_on_my_mothers_point_mass { 0, 0, 0 };

  // 1) Spring force -------------------------------------------------------------------
  //    Only the spring of this cylinder. The daughters spring also act on this mass,
  //    but they are treated in point (2)
  double factor = -tension_ / actual_length_;  // the minus sign is important because the spring axis goes in the opposite direction
  force_on_my_point_mass[0] += factor * spring_axis_[0];
  force_on_my_point_mass[1] += factor * spring_axis_[1];
  force_on_my_point_mass[2] += factor * spring_axis_[2];

  // 2) Force transmitted by daugthers (if they exist) ----------------------------------
  if (daughter_left_ != nullptr) {
    auto force_from_daughter = daughter_left_->forceTransmittedFromDaugtherToMother(
        std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
    force_on_my_point_mass[0] += force_from_daughter[0];
    force_on_my_point_mass[1] += force_from_daughter[1];
    force_on_my_point_mass[2] += force_from_daughter[2];
  }
  if (daughter_right_ != nullptr) {
    auto force_from_daughter = daughter_right_->forceTransmittedFromDaugtherToMother(
        std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
    force_on_my_point_mass[0] += force_from_daughter[0];
    force_on_my_point_mass[1] += force_from_daughter[1];
    force_on_my_point_mass[2] += force_from_daughter[2];
  }

  // 3) Object avoidance force -----------------------------------------------------------
  //  (We check for every neighbor object if they touch us, i.e. push us away)
  for (auto neighbor : so_node_->getNeighbors()) {
    // of course, only if it is an instance of PhysicalObject
    if (neighbor->isAPhysicalObject()) {
      auto n = std::static_pointer_cast<PhysicalObject>(neighbor);
      // if it is a direct relative, we don't take it into account
      if (neighbor == mother_ || neighbor == daughter_left_ || neighbor == daughter_right_)
        continue;
      // if sister branch, we also don't take into account
      if (n->isAPhysicalCylinder()) {
        auto n_cyl = std::static_pointer_cast<PhysicalCylinder>(neighbor);
        if (n_cyl->getMother() == mother_) {
          continue;
        }
      }
      // if we have a PhysicalBond with him, we also don't take it into account
      for (auto pb : physical_bonds_) {
        if (pb->getOppositePhysicalObject(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()))
            == neighbor) {
          continue;
        }
      }

      auto force_from_neighbor = n->getForceOn(std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));

      // 1) "artificial force" to maintain the sphere in the ecm simulation boundaries--------
      if (ecm_->getArtificialWallForCylinders()) {
        auto force_from_artificial_wall = ecm_->forceFromArtificialWall(mass_location_, diameter_ * 0.5);
        force_on_my_point_mass[0] += force_from_artificial_wall[0];
        force_on_my_point_mass[1] += force_from_artificial_wall[1];
        force_on_my_point_mass[2] += force_from_artificial_wall[2];
      }

      if (std::abs(force_from_neighbor[3]) < 1E-10) {
        // (if all the force is transmitted to the (distal end) point mass : )
        force_on_my_point_mass[0] += force_from_neighbor[0];
        force_on_my_point_mass[1] += force_from_neighbor[1];
        force_on_my_point_mass[2] += force_from_neighbor[2];
      } else {
        // (if there is a part transmitted to the proximal end : )
        double part_for_point_mass = 1.0 - force_from_neighbor[3];
        force_on_my_point_mass[0] += force_from_neighbor[0] * part_for_point_mass;
        force_on_my_point_mass[1] += force_from_neighbor[1] * part_for_point_mass;
        force_on_my_point_mass[2] += force_from_neighbor[2] * part_for_point_mass;
        force_on_my_mothers_point_mass[0] += force_from_neighbor[0] * force_from_neighbor[3];
        force_on_my_mothers_point_mass[1] += force_from_neighbor[1] * force_from_neighbor[3];
        force_on_my_mothers_point_mass[2] += force_from_neighbor[2] * force_from_neighbor[3];
      }
    }
  }

  bool anti_kink = false;
  // TEST : anti-kink
  if (anti_kink) {
    double KK = 5;
    if (daughter_left_ != nullptr && daughter_right_ == nullptr) {
      if (daughter_left_->daughter_left_ != nullptr) {
        auto downstream = daughter_left_->daughter_left_;
        double rresting = daughter_left_->getRestingLength() + downstream->getRestingLength();
        auto down_to_me = Matrix::subtract(mass_location_, downstream->mass_location_);
        double aactual = Matrix::norm(down_to_me);

        force_on_my_point_mass = Matrix::add(
            force_on_my_point_mass, Matrix::scalarMult(KK * (rresting - aactual), Matrix::normalize(down_to_me)));
      }
    }

    if (daughter_left_ != nullptr && mother_->isAPhysicalCylinder()) {
      auto mother_cyl = std::static_pointer_cast<PhysicalCylinder>(mother_);
      double rresting = getRestingLength() + mother_cyl->getRestingLength();
      auto down_to_me = Matrix::subtract(mass_location_, mother_cyl->proximalEnd());
      double aactual = Matrix::norm(down_to_me);

      force_on_my_point_mass = Matrix::add(
          force_on_my_point_mass, Matrix::scalarMult(KK * (rresting - aactual), Matrix::normalize(down_to_me)));
    }
  }
  // 4) PhysicalBond -----------------------------------------------------------
  for (auto pb : physical_bonds_) {
    auto force_physical_bond = pb->getForceOn(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));

    if (std::abs(force_physical_bond[3]) < 1E-10) {
      // (if all the force is transmitted to the (distal end) point mass : )
      force_on_my_point_mass[0] += force_physical_bond[0];
      force_on_my_point_mass[1] += force_physical_bond[1];
      force_on_my_point_mass[2] += force_physical_bond[2];
    } else {
      // (if there is a part transmitted to the proximal end : )
      double part_for_point_mass = 1.0 - force_physical_bond[3];
      force_on_my_point_mass[0] += force_physical_bond[0] * part_for_point_mass;
      force_on_my_point_mass[1] += force_physical_bond[1] * part_for_point_mass;
      force_on_my_point_mass[2] += force_physical_bond[2] * part_for_point_mass;
      force_on_my_mothers_point_mass[0] += force_physical_bond[0] * force_physical_bond[3];
      force_on_my_mothers_point_mass[1] += force_physical_bond[1] * force_physical_bond[3];
      force_on_my_mothers_point_mass[2] += force_physical_bond[2] * force_physical_bond[3];
    }
  }

  // 5) define the force that will be transmitted to the mother
  force_to_transmit_to_proximal_mass_ = force_on_my_mothers_point_mass;
  // 6) Compute the movement of this neurite elicited by the resultant force----------------
  //  6.0) In case we display the force
  total_force_last_time_step_[0] = force_on_my_point_mass[0];
  total_force_last_time_step_[1] = force_on_my_point_mass[1];
  total_force_last_time_step_[2] = force_on_my_point_mass[2];
  total_force_last_time_step_[3] = 1;
  //  6.1) Define movement scale
  double h_over_m = h / mass_;
  double force_norm = Matrix::norm(force_on_my_point_mass);
  //  6.2) If is F not strong enough -> no movements
  if (force_norm < adherence_) {
    total_force_last_time_step_[3] = -1;
    return;
  }
  // So, what follows is only executed if we do actually move :

  //  6.3) Since there's going be a move, we calculate it
  auto displacement = Matrix::scalarMult(h_over_m, force_on_my_point_mass);
  double displacement_norm = force_norm * h_over_m;

  //  6.4) There is an upper bound for the movement.
  if (displacement_norm > Param::kSimulationMaximalDisplacement) {
    displacement = Matrix::scalarMult(Param::kSimulationMaximalDisplacement / displacement_norm, displacement);
  }

  auto actual_displacement = displacement;

  // 8) Eventually, we do perform the move--------------------------------------------------
  // 8.1) The move of our mass
  mass_location_[0] += actual_displacement[0];
  mass_location_[1] += actual_displacement[1];
  mass_location_[2] += actual_displacement[2];
  // 8.2) Recompute length, tension and re-center the computation node, and redefine axis
  updateDependentPhysicalVariables();
  updateLocalCoordinateAxis();
  // 8.3) For the relatives: recompute the lenght, tension etc. (why for mother? have to think about that)
  if (daughter_left_ != nullptr) {
    daughter_left_->updateDependentPhysicalVariables();
    daughter_left_->updateLocalCoordinateAxis();
  }
  if (daughter_right_ != nullptr) {
    daughter_right_->updateDependentPhysicalVariables();
    daughter_right_->updateLocalCoordinateAxis();
  }
  // 8.4) next time we reschedule everyone :
  scheduleMeAndAllMyFriends();
}

std::array<double, 3> PhysicalCylinder::getForceOn(const std::shared_ptr<PhysicalSphere>& s) {
  return inter_object_force_->forceOnASphereFromACylinder(
      s, std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));
}

std::array<double, 4> PhysicalCylinder::getForceOn(const std::shared_ptr<PhysicalCylinder>& c) {
  if (c->getMother() == mother_) {
    // extremely important to avoid that two sister branches start to
    // interact physically.
    return {0.0, 0.0, 0.0, 0.0};
  }
  return inter_object_force_->forceOnACylinderFromACylinder(
      c, std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));

}

bool PhysicalCylinder::isInContactWithSphere(const std::shared_ptr<PhysicalSphere>& s) {
  auto force = inter_object_force_->forceOnACylinderFromASphere(
      std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()), s);
  return Matrix::norm(force) > 1E-15;
}

bool PhysicalCylinder::isInContactWithCylinder(const std::shared_ptr<PhysicalCylinder>& c) {
  auto force = inter_object_force_->forceOnACylinderFromACylinder(
      std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()), c);
  return Matrix::norm(force) > 1E-15;
}

std::array<double, 3> PhysicalCylinder::closestPointTo(const std::array<double, 3>& p) {
  auto mass_to_p = Matrix::subtract(p, mass_location_);
  double mass_to_p_dot_minus_axis = -mass_to_p[0] * spring_axis_[0] - mass_to_p[1] * spring_axis_[1]
      - mass_to_p[2] * spring_axis_[2];
  double K = mass_to_p_dot_minus_axis / (actual_length_ * actual_length_);

  std::array<double, 3> cc { 0, 0, 0 };  // the closest point
  if (K <= 1.0 && K >= 0.0) {
    cc = {mass_location_[0]-K*spring_axis_[0], mass_location_[1]-K*spring_axis_[1], mass_location_[2]-K*spring_axis_[2]};
  } else if(K<0) {
    cc = mass_location_;
  } else {
    cc = proximalEnd();
  }
  return cc;
}

void PhysicalCylinder::runIntracellularDiffusion() {
// 1) Degradation according to the degradation constant for each chemical
  for (auto el : intracellular_substances_) {
    auto s = el.second;
    double decay = ecm_->exp(-s->getDegradationConstant() * Param::kSimulationTimeStep);
    s->multiplyQuantityAndConcentrationBy(decay);
  }

// 2) each cylinder is responsible for diffusion with its distal relatives (i.e daughter left
// and daughter right). The direction (i.e.who calls diffuseWithThisPhysicalObject() )
// is chosen randomly. To be sure that new substances will be transmitted bi-directionally.
// For now we start first with dL and then dR. This might have to change...
  auto daughter_right = daughter_right_;
  auto daughter_left = daughter_left_;

  if (daughter_right != nullptr) {
    auto po_1 = std::static_pointer_cast<PhysicalObject>(this->shared_from_this());
    std::shared_ptr<PhysicalObject> po_2 = daughter_right;
    if (ecm_->getRandomDouble1() < 0.5) {
      po_1 = daughter_right;
      po_2 = std::static_pointer_cast<PhysicalObject>(this->shared_from_this());
    }
    po_1->diffuseWithThisPhysicalObjects(po_2, daughter_right->getActualLength());
  }

  if (daughter_left != nullptr) {
    auto po_1 = std::static_pointer_cast<PhysicalObject>(this->shared_from_this());
    std::shared_ptr<PhysicalObject> po_2 = daughter_left;
    if (ecm_->getRandomDouble1() < 0.5) {
      po_1 = daughter_left;
      po_2 = std::static_pointer_cast<PhysicalObject>(this->shared_from_this());
    }
    po_1->diffuseWithThisPhysicalObjects(po_2, daughter_left->getActualLength());
  }

}

std::array<double, 3> PhysicalCylinder::getUnitNormalVector(const std::array<double, 3>& position) const {
  return Matrix::add(Matrix::scalarMult(ecm_->cos(position[1]), y_axis_),
                     Matrix::scalarMult(ecm_->sin(position[1]), z_axis_));
}

void PhysicalCylinder::updateLocalCoordinateAxis() {
  // x (new) = something new
  // z (new) = x (new) cross y(old)
  // y (new) = z(new) cross x(new)
  x_axis_ = Matrix::normalize(spring_axis_);
  z_axis_ = Matrix::crossProduct(x_axis_, y_axis_);
  double norm_of_z = Matrix::norm(z_axis_);
  if (norm_of_z < 1E-10) {
    // If new x_axis_ and old y_axis_ are aligned, we cannot use this scheme;
    // we start by re-defining new perp vectors. Ok, we loose the previous info, but
    // this should almost never happen....
    z_axis_ = Matrix::perp3(x_axis_, ecm_->matrixNextRandomDouble());
  } else {
    z_axis_ = Matrix::scalarMult((1 / norm_of_z), z_axis_);
  }
  y_axis_ = Matrix::crossProduct(z_axis_, x_axis_);
}

void PhysicalCylinder::updateDiameter() {
  diameter_ = ecm_->sqrt(volume_ * 1.27323954 / actual_length_);   // 1.27323 = 4/pi
}

void PhysicalCylinder::updateVolume() {           // 0.78539 = pi/4
  volume_ = 0.785398163 * diameter_ * diameter_ * actual_length_;
  updateIntracellularConcentrations();
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const {
  auto pos = Matrix::subtract(position, proximalEnd());
  return {
    Matrix::dot(pos,x_axis_),
    Matrix::dot(pos,y_axis_),
    Matrix::dot(pos,z_axis_)
  };
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesLocalToGlobal(const std::array<double, 3>& position) const {
  std::array<double, 3> glob { position[0] * x_axis_[0] + position[1] * y_axis_[0] + position[2] * z_axis_[0],
      position[0] * x_axis_[1] + position[1] * y_axis_[1] + position[2] * z_axis_[1], position[0] * x_axis_[2]
          + position[1] * y_axis_[2] + position[2] * z_axis_[2] };
  return Matrix::add(glob, proximalEnd());
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesLocalToPolar(const std::array<double, 3>& position) const {
  return {
    position[0],
    ecm_->atan2(position[2], position[1]),
    ecm_->sqrt(position[1]*position[1] + position[2]*position[2])
  };
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesPolarToLocal(const std::array<double, 3>& position) const {
  return {
    position[0],
    position[2]*ecm_->cos(position[1]),
    position[2]*ecm_->sin(position[1])
  };
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesPolarToGlobal(const std::array<double, 2>& position) const {
  // the positionInLocalCoordinate is in cylindrical coord (h,theta,r)
  // with r being implicit (half the diameter_)
  // We thus have h (along x_axis_) and theta (the angle from the y_axis_).
  double r = 0.5 * diameter_;
  std::array<double, 3> polar_position { position[0], position[1], r };
  auto local = transformCoordinatesPolarToLocal(polar_position);
  return transformCoordinatesLocalToGlobal(local);
}

std::array<double, 3> PhysicalCylinder::transformCoordinatesGlobalToPolar(const std::array<double, 3>& position) const {
  auto local = transformCoordinatesGlobalToLocal(position);
  return transformCoordinatesLocalToPolar(local);
}

std::shared_ptr<local_biology::CellElement> PhysicalCylinder::getCellElement() const {
  return neurite_element_;
}

std::shared_ptr<local_biology::NeuriteElement> PhysicalCylinder::getNeuriteElement() const {
  return neurite_element_;
}

void PhysicalCylinder::setNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite_element) {
  if (neurite_element != nullptr) {
    neurite_element_ = neurite_element;
  } else {
    throw std::logic_error("ERROR  PhysicalCylinder: neuriteElement already exists");
  };
}

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::getDaughterLeft() const {
  return daughter_left_;
}

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::getDaughterRight() const {
  return daughter_right_;
}

std::shared_ptr<PhysicalObject> PhysicalCylinder::getMother() const {
  return mother_;
}

void PhysicalCylinder::setMother(const std::shared_ptr<PhysicalObject>& m) {
  mother_ = m;
}

void PhysicalCylinder::setDaughterLeft(const std::shared_ptr<PhysicalCylinder>& daughter) {
  daughter_left_ = daughter;
}

void PhysicalCylinder::setDaughterRight(const std::shared_ptr<PhysicalCylinder>& daughter) {
  daughter_right_ = daughter;
}

void PhysicalCylinder::setBranchOrder(int branch_order) {
  branch_order_ = branch_order;
}

int PhysicalCylinder::getBranchOrder() const {
  return branch_order_;
}

double PhysicalCylinder::getActualLength() const {
  return actual_length_;
}

void PhysicalCylinder::setActualLength(double actual_length) {
  actual_length_ = actual_length;
}

double PhysicalCylinder::getRestingLength() const {
  return resting_length_;
}

void PhysicalCylinder::setRestingLength(double resting_length) {
  resting_length_ = resting_length;
}

std::array<double, 3> PhysicalCylinder::getSpringAxis() const {
  return spring_axis_;
}

void PhysicalCylinder::setSpringAxis(const std::array<double, 3>& axis) {
  spring_axis_ = axis;
}

double PhysicalCylinder::getSpringConstant() const {
  return spring_constant_;
}

void PhysicalCylinder::setSpringConstant(double spring_constant) {
  spring_constant_ = spring_constant;
}

double PhysicalCylinder::getTension() const {
  return tension_;
}

void PhysicalCylinder::setTension(double t) {
  tension_ = t;
}

std::array<double, 3> PhysicalCylinder::getUnitaryAxisDirectionVector() const {
  double factor = 1.0 / actual_length_;
  return {factor*spring_axis_[0], factor*spring_axis_[1], factor*spring_axis_[2]};
}

bool PhysicalCylinder::isTerminal() const {
  return (daughter_left_ == nullptr);
}

bool PhysicalCylinder::bifurcationPermitted() const {
  return (daughter_left_ == nullptr && actual_length_ > Param::kNeuriteMinimalBifurcationLength);
}

bool PhysicalCylinder::branchPermitted() const {
  return (daughter_left_ != nullptr && daughter_right_ == nullptr);
}

std::array<double, 3> PhysicalCylinder::proximalEnd() const {
  return Matrix::subtract(mass_location_, spring_axis_);
}

std::array<double, 3> PhysicalCylinder::distalEnd() const {
  return mass_location_;
}

double PhysicalCylinder::lengthToProximalBranchingPoint() const {
  double length = actual_length_;
  if (mother_->isAPhysicalCylinder()) {
    auto previous_cylinder = std::static_pointer_cast<PhysicalCylinder>(mother_);
    if (previous_cylinder->getDaughterRight() == nullptr) {
      length += previous_cylinder->lengthToProximalBranchingPoint();
    }
  }
  return length;
}

bool PhysicalCylinder::isAPhysicalCylinder() const {
  return true;
}

double PhysicalCylinder::getLength() const {
  return actual_length_;
}

double PhysicalCylinder::getInterObjectForceCoefficient() const {
  // TODO Auto-generated method stub
  return 0;
}

void PhysicalCylinder::setInterObjectForceCoefficient(double coefficient) {
  // TODO Auto-generated method stub
}

std::array<double, 3> PhysicalCylinder::getAxis() const {
  return x_axis_;
}

void PhysicalCylinder::updateSpatialOrganizationNodePosition() {

  auto current_spatial_node_position = so_node_->getPosition();

  std::array<double, 3> center_displacement { mass_location_[0] - 0.5 * spring_axis_[0]
      - current_spatial_node_position[0], mass_location_[1] - 0.5 * spring_axis_[1] - current_spatial_node_position[1],
      mass_location_[2] - 0.5 * spring_axis_[2] - current_spatial_node_position[2] };
  double diameter = diameter_;
  // to save time in SOM operation, if the displacement is very small, we don't do it
  if (Matrix::norm(center_displacement) < diameter / 4.0)
    return;
  // To avoid perfect alignment (pathologic position for Delaunay, eg), we Matrix::add a small jitter
  // TODO remove next line when we have Dennis'stable Delaunay
  center_displacement = Matrix::add(center_displacement, ecm_->matrixRandomNoise3(diameter / 4.0));

  // Tell the node to moves
  so_node_->moveFrom(center_displacement);  // todo catch PositionNotAllowedException
}

void PhysicalCylinder::updateDependentPhysicalVariables() {
  auto relative_ml = mother_->originOf(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));  // mass_location_ of the mother
  spring_axis_[0] = mass_location_[0] - relative_ml[0];
  spring_axis_[1] = mass_location_[1] - relative_ml[1];
  spring_axis_[2] = mass_location_[2] - relative_ml[2];
  actual_length_ = ecm_->sqrt(
      spring_axis_[0] * spring_axis_[0] + spring_axis_[1] * spring_axis_[1] + spring_axis_[2] * spring_axis_[2]);
  tension_ = spring_constant_ * (actual_length_ - resting_length_) / resting_length_;
  updateVolume();
  updateSpatialOrganizationNodePosition();

}

void PhysicalCylinder::updateIntracellularConcentrations() {
  for (auto el : intracellular_substances_) {
    auto s = el.second;
    if (s->isVolumeDependant()) {
      s->updateConcentrationBasedOnQuantity(volume_);
    } else {
      s->updateConcentrationBasedOnQuantity(actual_length_);
    }
  }
}

std::shared_ptr<local_biology::NeuriteElement> PhysicalCylinder::insertProximalCylinder() {
  return insertProximalCylinder(0.5);
}

std::shared_ptr<local_biology::NeuriteElement> PhysicalCylinder::insertProximalCylinder(double distal_portion) {
  // location
  std::array<double, 3> new_mass_location { mass_location_[0] - distal_portion * spring_axis_[0], mass_location_[1]
      - distal_portion * spring_axis_[1], mass_location_[2] - distal_portion * spring_axis_[2] };
  double temp = distal_portion + (1 - distal_portion) / 2.0;
  std::array<double, 3> newProximalCylinderSpatialNodeLocation { mass_location_[0] - temp * spring_axis_[0],
      mass_location_[1] - temp * spring_axis_[1], mass_location_[2] - temp * spring_axis_[2] };
  // creating a new PhysicalCylinder & a new NeuriteElement, linking them together
  auto new_cylinder = getCopy();
  auto ne = neurite_element_->getCopy();
  ne->setPhysical(new_cylinder);
  new_cylinder->mass_location_ = new_mass_location;
  // familly relations
  mother_->updateRelative(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()), new_cylinder);
  new_cylinder->setMother(getMother());
  setMother(new_cylinder);
  new_cylinder->setDaughterLeft(std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));
  // SOM relation
  auto new_son = so_node_->getNewInstance(newProximalCylinderSpatialNodeLocation, new_cylinder);  // todo catch PositionNotAllowedException
  new_cylinder->setSoNode(new_son);
  // registering the new cylinder with ecm
  ecm_->addPhysicalCylinder(new_cylinder);
  // physics
  new_cylinder->resting_length_ = (1 - distal_portion) * resting_length_;
  resting_length_ *= distal_portion;

  // intracellularSubstances quantities .....................................
  // (concentrations are solved in updateDependentPhysicalVariables():
  for (auto pair : intracellular_substances_) {
    auto s = pair.second;
    // if doesn't diffuse at all : all the substance stays in the distal part !
    if (s->getDiffusionConstant() < 0.000000000001) {
      continue;
    }
    // create similar IntracellularSubstance and insert it into the new cylinder
    double quantity_before_distribution = s->getQuantity();
    auto s2 = IntracellularSubstance::create(s);
    s2->setQuantity(quantity_before_distribution * (1 - distal_portion));
    new_cylinder->addNewIntracellularSubstance(s2);
    // decrease value of IntracellularSubstance in this cylinder
    s->setQuantity(quantity_before_distribution * distal_portion);
  }
  updateDependentPhysicalVariables();
  new_cylinder->updateDependentPhysicalVariables();
  new_cylinder->updateLocalCoordinateAxis();  // has to come after updateDepend...

  // copy the LocalBiologicalModules (not done in NeuriteElement, because this creation of
  // cylinder-neuriteElement is decided for physical and not biological reasons
  for (auto module : neurite_element_->getLocalBiologyModulesList()) {
    if (module->isCopiedWhenNeuriteElongates())
      ne->addLocalBiologyModule(module->getCopy());
  }

  // deal with the excressences:
  if (!excrescences_.empty()) {
    auto it = excrescences_.begin();
    do {
      auto ex = *it;
      auto pos = ex->getPositionOnPO();
      // transmit them to proximal cyl
      if (pos[0] < new_cylinder->actual_length_) {
        excrescences_.remove(ex);
        new_cylinder->addExcrescence(ex);
        ex->setPo(new_cylinder);
        it--;
      } else {
        // or kep them here, depending on coordinate
        pos[0] -= new_cylinder->actual_length_;
        ex->setPositionOnPO(pos);
      }
    } while (++it != excrescences_.end());
  }
  return ne;
}

void PhysicalCylinder::removeProximalCylinder() {
  // The mother is removed if (a) it is a PhysicalCylinder and (b) it has no other daughter than
  if (!mother_->isAPhysicalCylinder()
      || std::static_pointer_cast<PhysicalCylinder>(mother_)->getDaughterRight() != nullptr) {
    return;
  }
  // The guy we gonna remove
  auto proximal_cylinder = std::static_pointer_cast<PhysicalCylinder>(mother_);
  // the ex-mother's neurite Element has to be removed
  proximal_cylinder->getNeuriteElement()->removeYourself();
  // Re-organisation of the PhysicalObject tree structure: by-passing proximalCylinder
  proximal_cylinder->getMother()->updateRelative(proximal_cylinder,
                                                 std::static_pointer_cast<PhysicalCylinder>(this->shared_from_this()));
  setMother(proximal_cylinder->getMother());

  // collecting (the quantities of) the intracellular substances of the removed cylinder.
  for (auto s : proximal_cylinder->getIntracellularSubstances1()) {
    modifyIntracellularQuantity(s->getId(), s->getQuantity() / Param::kSimulationTimeStep);
    // divided by time step, because in the method the parameter is multiplied by time step...
    // and we want to change the quantity.
    // We don't change the concentration, it is done later by the call to updateVolume()
  }

  // Keeping the same tension :
  // (we don't use updateDependentPhysicalVariables(), because we have tension and want to
  // compute restingLength, and not the opposite...)
  // T = k*(A-R)/R --> R = k*A/(T+K)
  spring_axis_ = Matrix::subtract(
      mass_location_, mother_->originOf(std::static_pointer_cast<PhysicalObject>(this->shared_from_this())));
  actual_length_ = Matrix::norm(spring_axis_);
  resting_length_ = spring_constant_ * actual_length_ / (tension_ + spring_constant_);
  // .... and volume_
  updateVolume();
  // and local coord
  updateLocalCoordinateAxis();
  // ecm
  ecm_->removePhysicalCylinder(proximal_cylinder);

  // dealing with excressences:
  // mine are shifted up :
  double shift = actual_length_ - proximal_cylinder->actual_length_;
  for (auto ex : excrescences_) {
    auto pos = ex->getPositionOnPO();
    pos[0] += shift;
    ex->setPositionOnPO(pos);
  }
  // I incorporate the ones of the previous cyl:
  for (auto ex : proximal_cylinder->excrescences_) {
    excrescences_.push_back(ex);
    ex->setPo(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
  }
  // TODO: take care of Physical Bonds
  proximal_cylinder->setStillExisting(false);
  // the SON
  updateSpatialOrganizationNodePosition();
  // TODO: CAUTION : for future parallel implementation. If a visitor is in the branch, it gets destroyed....
}

void PhysicalCylinder::scheduleMeAndAllMyFriends() {
  // me
  setOnTheSchedulerListForPhysicalObjects(true);
  // relatives :
  mother_->setOnTheSchedulerListForPhysicalObjects(true);
  if (daughter_left_ != nullptr) {
    daughter_left_->setOnTheSchedulerListForPhysicalObjects(true);
    if (daughter_right_ != nullptr) {
      daughter_right_->setOnTheSchedulerListForPhysicalObjects(true);
    }
  }
  // neighbors in the triangulation :
  for (auto neighbor : so_node_->getNeighbors()) {
    if (neighbor->isAPhysicalObject()) {
      std::static_pointer_cast<PhysicalObject>(neighbor)->setOnTheSchedulerListForPhysicalObjects(true);
    }
  }
  for (auto bond : physical_bonds_) {
    bond->getOppositePhysicalObject(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()))
        ->setOnTheSchedulerListForPhysicalObjects(true);
  }
}

std::shared_ptr<PhysicalCylinder> PhysicalCylinder::extendSideCylinder(double length,
                                                                       const std::array<double, 3>& direction) {
  auto new_branch = getCopy();
  auto dir = direction;
  // TODO : better method !
  double angle_with_side_branch = Matrix::angleRadian(spring_axis_, direction);
  if (angle_with_side_branch < 0.78 || angle_with_side_branch > 2.35) {  // 45-135 degrees
    auto p = Matrix::crossProduct(spring_axis_, direction);
    p = Matrix::crossProduct(p, spring_axis_);
    dir = Matrix::add(Matrix::normalize(direction), Matrix::normalize(p));
  }
  // location of mass and computation center
  auto new_spring_axis = Matrix::scalarMult(length, Matrix::normalize(dir));
  auto new_mass_location = Matrix::add(mass_location_, new_spring_axis);
  new_branch->mass_location_ = new_mass_location;
  new_branch->spring_axis_ = new_spring_axis;
  // physics
  new_branch->actual_length_ = length;
  new_branch->setRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  new_branch->setDiameter(Param::kNeuriteDefaultDiameter, true);
  new_branch->updateLocalCoordinateAxis();
  // family relations
  new_branch->setMother(std::static_pointer_cast<PhysicalObject>(this->shared_from_this()));
  daughter_right_ = new_branch;
  // new CentralNode
  auto new_center_location = Matrix::add(mass_location_, Matrix::scalarMult(0.5, new_spring_axis));
  auto new_son = so_node_->getNewInstance(new_center_location, new_branch);  // todo catch PositionNotAllowedException
  new_branch->setSoNode(new_son);
  // correct physical values (has to be after family relations and SON assignement).
  new_branch->updateDependentPhysicalVariables();
  // register to ecm
  ecm_->addPhysicalCylinder(new_branch);

  // i'm scheduled to run physics next time :
  // (the side branch automatically is too, because it's a new PhysicalObject)
  setOnTheSchedulerListForPhysicalObjects(true);
  return new_branch;
}

}  // namespace physics
}  // namespace cx3d
