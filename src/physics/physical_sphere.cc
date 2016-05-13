#include "physics/physical_sphere.h"

#include <sstream>
#include <cmath>
#include <exception>

#include "param.h"
#include "matrix.h"
#include "stl_util.h"
#include "string_util.h"
#include "sim_state_serialization_util.h"

#include "physics/physical_bond.h"
#include "physics/inter_object_force.h"
#include "physics/intracellular_substance.h"
#include "physics/debug/physical_sphere_debug.h"

#include "spatial_organization/space_node.h"

#include "local_biology/cell_element.h"
#include "local_biology/soma_element.h"

namespace cx3d {
namespace physics {

std::shared_ptr<PhysicalSphere> PhysicalSphere::create() {
  return std::shared_ptr<PhysicalSphere>(new PhysicalSphere());
//  return std::shared_ptr<PhysicalSphere>(new PhysicalSphereDebug());
}

PhysicalSphere::PhysicalSphere() {
  mass_ = Param::kSphereDefaultMass;
  adherence_ = Param::kSphereDefaultAdherence;
  diameter_ = Param::kSphereDefaultDiameter;
  updateVolume();
}

StringBuilder& PhysicalSphere::simStateToJson(StringBuilder& sb) const {
  PhysicalObject::simStateToJson(sb);

  // somaElement is circular reference;
  SimStateSerializationUtil::unorderedCollection(sb, "daughters", daughters_);
  SimStateSerializationUtil::mapOfDoubleArray(sb, "daughtersCoord", daughters_coord_);
  SimStateSerializationUtil::keyValue(sb, "rotationInertia", rotational_inertia_);
  SimStateSerializationUtil::keyValue(sb, "interObjectForceCoefficient", inter_object_force_coefficient_);
  SimStateSerializationUtil::keyValue(sb, "tractorForce", tractor_force_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

std::string PhysicalSphere::toString() const {
  std::ostringstream str_stream;
  str_stream << "(";
  str_stream << StringUtil::toStr(getID());
//  str_stream << StringUtil::toStr(mass_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(volume_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(adherence_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(total_force_last_time_step_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(physical_bonds_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(x_axis_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(y_axis_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(z_axis_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(diameter_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(mass_location_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(daughters_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(daughters_coord_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(rotational_inertia_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(inter_object_force_coefficient_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(tractor_force_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(soma_element_);
  str_stream << ")";
  return str_stream.str();
}

double PhysicalSphere::getInterObjectForceCoefficient() const {
  return inter_object_force_coefficient_;
}

void PhysicalSphere::setInterObjectForceCoefficient(double inter_object_force_coefficient) {
  if (inter_object_force_coefficient < 0.0) {
    inter_object_force_coefficient = 0.0;
  } else if (inter_object_force_coefficient > 1.0) {
    inter_object_force_coefficient = 1.0;
  }
  inter_object_force_coefficient_ = inter_object_force_coefficient;
}

double PhysicalSphere::getRotationalInertia() const {
  return rotational_inertia_;
}

void PhysicalSphere::setRotationalInertia(double rotational_inertia) {
  rotational_inertia_ = rotational_inertia;
}

bool PhysicalSphere::isAPhysicalSphere() const {
  return true;
}

void PhysicalSphere::movePointMass(double speed, const std::array<double, 3>& direction) {
  // NOTE :
  // a) division by norm(direction), so we get a pure direction
  // b) multiplication by the mass, because the total force is divide
  //    by the mass in runPhysics().
  // c) multiplication by speed for obvious reasons...
  // d) the scaling for simulation time step occurs in the runPhysics() method

  double n = Matrix::norm(direction);
  if (n == 0)
    return;
  double factor = speed / n;

  tractor_force_[0] = factor * (direction[0]);
  tractor_force_[1] = factor * (direction[1]);
  tractor_force_[2] = factor * (direction[2]);
  // if we are told to move (by our SomaElement for instance), we will update
  // our physics.
  if (speed > 1E-10) {
    PhysicalObject::setOnTheSchedulerListForPhysicalObjects(true);
  }
}

std::array<double, 3> PhysicalSphere::originOf(const std::shared_ptr<PhysicalObject>& daughter_asking) {
  std::array<double, 3> xyz = daughters_coord_[std::static_pointer_cast<PhysicalCylinder>(daughter_asking)];

  double radius = diameter_ * .5;
  xyz = Matrix::scalarMult(radius, xyz);

  return {
    mass_location_[0] + xyz[0] * x_axis_[0] + xyz[1] * y_axis_[0] + xyz[2] * z_axis_[0],
    mass_location_[1] + xyz[0] * x_axis_[1] + xyz[1] * y_axis_[1] + xyz[2] * z_axis_[1],
    mass_location_[2] + xyz[0] * x_axis_[2] + xyz[1] * y_axis_[2] + xyz[2] * z_axis_[2]
  };
}

SomaElement* PhysicalSphere::getSomaElement() const {
  return soma_element_;
}

void PhysicalSphere::setSomaElement(SomaElement* soma_element) {
  if (soma_element != nullptr) {
    soma_element_ = soma_element;
  } else {
    throw std::logic_error("ERROR  PhysicalSphere: somaElement already exists");
  }
}

void PhysicalSphere::changeVolume(double speed) {
  //scaling for integration step
  double dV = speed * Param::kSimulationTimeStep;
  volume_ += dV;
  if (volume_ < 5.2359877E-7) {  // minimum volume, corresponds to minimal diameter
    volume_ = 5.2359877E-7;
  }
  updateDiameter();
  updateIntracellularConcentrations();
  scheduleMeAndAllMyFriends();
}

void PhysicalSphere::changeDiameter(double speed) {
  //scaling for integration step
  double dD = speed * Param::kSimulationTimeStep;
  diameter_ += dD;
  if (diameter_ < 0.01) {
    diameter_ = 0.01;  // minimum diameter
  }
  updateVolume();
  // no call to updateIntracellularConcentrations() cause it's done by updateVolume().
  scheduleMeAndAllMyFriends();
}

std::shared_ptr<PhysicalCylinder> PhysicalSphere::addNewPhysicalCylinder(double new_length, double phi, double theta) {
  double radius = 0.5 * diameter_;
  // position in cx3d.cells coord
  double x_coord = ecm_->cos(theta) * ecm_->sin(phi);
  double y_coord = ecm_->sin(theta) * ecm_->sin(phi);
  double z_coord = ecm_->cos(phi);
  std::array<double, 3> axis_direction { x_coord * x_axis_[0] + y_coord * y_axis_[0] + z_coord * z_axis_[0], x_coord
      * x_axis_[1] + y_coord * y_axis_[1] + z_coord * z_axis_[1], x_coord * x_axis_[2] + y_coord * y_axis_[2]
      + z_coord * z_axis_[2] };

  // positions & axis in cartesian coord
  auto new_cyl_begin_location = Matrix::add(mass_location_, Matrix::scalarMult(radius, axis_direction));
  auto new_cyl_spring_axis = Matrix::scalarMult(new_length, axis_direction);

  auto new_cyl_mass_location = Matrix::add(new_cyl_begin_location, new_cyl_spring_axis);
  auto new_cyl_central_node_location = Matrix::add(new_cyl_begin_location,
                                                   Matrix::scalarMult(0.5, new_cyl_spring_axis));
  // new PhysicalCylinder
  auto cyl = ecm_->newPhysicalCylinder();
  cyl->setSpringAxis(new_cyl_spring_axis);
  cyl->setMassLocation(new_cyl_mass_location);
  cyl->setActualLength(new_length);
  cyl->setRestingLengthForDesiredTension(Param::kNeuriteDefaultTension);
  cyl->updateLocalCoordinateAxis();
  cyl->setDiameter(Param::kNeuriteDefaultDiameter, true);
  cyl->setColor(color_);
  // family relations
  daughters_.push_back(cyl);
  cyl->setMother(std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
  daughters_coord_[cyl] = {x_coord, y_coord, z_coord};

  // SpaceNode
  auto new_son = PhysicalObject::so_node_->getNewInstance(new_cyl_central_node_location, cyl);  // fixme catch PositionNotAllowedException

  cyl->setSoNode(new_son);
  PhysicalNode::ecm_->addPhysicalCylinder(cyl);
  return cyl;
}

std::shared_ptr<PhysicalSphere> PhysicalSphere::divide(double vr, double phi, double theta) {
  // A) Defining some values ..................................................................
  double old_volume = volume_;
  // defining the two radii s.t total volume is conserved ( R^3 = r1^3 + r2^3 ; vr = r2^3 / r1^3 )
  double radius = diameter_ * 0.5;
  double r1 = radius / std::pow(1.0 + vr, 1.0 / 3.0);
  double r2 = radius / std::pow(1.0 + 1 / vr, 1.0 / 3.0);
  // define an axis for division (along which the nuclei will move) in cx3d.cells Coord
  double x_coord = ecm_->cos(theta) * ecm_->sin(phi);
  double y_coord = ecm_->sin(theta) * ecm_->sin(phi);
  double z_coord = ecm_->cos(phi);
  double total_length_of_displacement = radius / 4.0;
  std::array<double, 3> axis_of_division { total_length_of_displacement
      * (x_coord * x_axis_[0] + y_coord * y_axis_[0] + z_coord * z_axis_[0]), total_length_of_displacement
      * (x_coord * x_axis_[1] + y_coord * y_axis_[1] + z_coord * z_axis_[1]), total_length_of_displacement
      * (x_coord * x_axis_[2] + y_coord * y_axis_[2] + z_coord * z_axis_[2]) };
  // two equations for the center displacement :
  //  1) d2/d1= v2/v1 = vr (each sphere is shifted inver. proportionally to its volume)
  //  2) d1 + d2 = TOTAL_LENGTH_OF_DISPLACEMENT
  double d_2 = total_length_of_displacement / (vr + 1);
  double d_1 = total_length_of_displacement - d_2;

  // B) Instantiating a new sphere = 2nd daughter................................................
  // getting a new sphere
  auto new_sphere = ecm_->newPhysicalSphere();  // change to create(); once porting has been finished

  // super class variables (except masLocation, filled below)
  new_sphere->setXAxis(x_axis_);
  new_sphere->setYAxis(y_axis_);
  new_sphere->setZAxis(z_axis_);
  new_sphere->setColor(color_);
  new_sphere->setAdherence(adherence_);
  new_sphere->setMass(mass_);
  new_sphere->setStillExisting(isStillExisting());

  // this class variables (except radius/diameter)
  new_sphere->rotational_inertia_ = rotational_inertia_;
  new_sphere->adherence_ = adherence_;
  new_sphere->setInterObjectForceCoefficient(inter_object_force_coefficient_);
  new_sphere->diameter_ = r2 * 2;
  new_sphere->volume_ = 0.523598776 * new_sphere->diameter_ * new_sphere->diameter_ * new_sphere->diameter_;  // 0,523598776 = (4/3)*pi*(1/(2^3))

  // Mass Location
  std::array<double, 3> new_mass_location { mass_location_[0] + d_2 * axis_of_division[0], mass_location_[1]
      + d_2 * axis_of_division[1], mass_location_[2] + d_2 * axis_of_division[2] };
  new_sphere->setMassLocation(new_mass_location);

  // C) Request a SpaceNode

  auto new_son = so_node_->getNewInstance(new_mass_location, new_sphere);  //fixme catch PositionNotAllowedException catch exception
  new_sphere->setSoNode(new_son);

  // D) register new Sphere to ECM
  ecm_->addPhysicalSphere(new_sphere);  // this method also adds the PhysicalNode

  // E) This sphere becomes the 1st daughter.....................................................
  // move this cx3d.cells on opposite direction (move the centralNode & the massLocation)
  so_node_->moveFrom( { -d_1 * axis_of_division[0], -d_1 * axis_of_division[1], -d_1 * axis_of_division[2] });  //fixme catch PositionNotAllowedException catch exception
  mass_location_[0] -= d_1 * axis_of_division[0];
  mass_location_[1] -= d_1 * axis_of_division[1];
  mass_location_[2] -= d_1 * axis_of_division[2];

  // F) change properties of this cell
  diameter_ = r1 * 2;
  volume_ = 0.523598776 * diameter_ * diameter_ * diameter_;  // 0,523598776 = (4/3)*pi*(1/(2^3))

  // G) Copy the intracellular and membrane bound Substances
  for (auto el : intracellular_substances_) {
    auto sub = el.second;
    auto sub_copy = std::static_pointer_cast<IntracellularSubstance>(sub->getCopy());   //copy substance
    sub->distributeConcentrationOnDivision(sub_copy);
    sub->updateQuantityBasedOnConcentration(volume_);
    sub_copy->updateQuantityBasedOnConcentration(new_sphere->volume_);
    new_sphere->intracellular_substances_[sub_copy->getId()] = sub_copy;
  }
  return new_sphere;
}

bool PhysicalSphere::isInContactWithSphere(const std::shared_ptr<PhysicalSphere>& s) {
  auto force = inter_object_force_->forceOnASphereFromASphere(
      std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()), s);
  return Matrix::norm(force) > 1E-15;
}

bool PhysicalSphere::isInContactWithCylinder(const std::shared_ptr<PhysicalCylinder>& c) {
  auto force = inter_object_force_->forceOnACylinderFromASphere(
      c, std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
  return Matrix::norm(force) > 1E-15;
}

std::array<double, 4> PhysicalSphere::getForceOn(const std::shared_ptr<PhysicalCylinder>& c) {
// get force on a Cylinder from a sphere
  return inter_object_force_->forceOnACylinderFromASphere(
      c, std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
}

std::array<double, 3> PhysicalSphere::getForceOn(const std::shared_ptr<PhysicalSphere>& s) {
  return inter_object_force_->forceOnASphereFromASphere(
      s, std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
}

void PhysicalSphere::runPhysics() {
  // Basically, the idea is to make the sum of all the forces acting
  // on the Point mass. It is stored in translationForceOnPointMass.
  // There is also a computation of the torque (only applied
  // by the daughter neurites), stored in rotationForce.

  // TODO : There might be a problem, in the sense that the biology is not applied
  // if the total Force is smaller than adherence. Once, I should look at this more carefully.

  // If we detect enough forces to make us  move, we will re-schedule us
  setOnTheSchedulerListForPhysicalObjects(false);

  //fixme why? copying
  auto xAxis = getXAxis();
  auto yAxis = getYAxis();
  auto zAxis = getZAxis();
  auto tf = tractor_force_;

  // the 3 types of movement that can occur
  bool biological_translation = false;
  bool physical_translation = false;
  bool physical_rotation = false;

  double h = Param::kSimulationTimeStep;
  std::array<double, 3> movement_at_next_step { 0, 0, 0 };

  // BIOLOGY :
  // 0) Start with tractor force : What the biology defined as active movement------------
  movement_at_next_step[0] += h * tf[0];
  movement_at_next_step[1] += h * tf[1];
  movement_at_next_step[2] += h * tf[2];

  // PHYSICS
  // the physics force to move the point mass
  std::array<double, 3> translation_force_on_point_mass { 0, 0, 0 };
  // the physics force to rotate the cell
  std::array<double, 3> rotation_force { 0, 0, 0 };

  // 1) "artificial force" to maintain the sphere in the ecm simulation boundaries--------
  if (ecm_->getArtificialWallForSpheres()) {
    auto force_from_artificial_wall = ecm_->forceFromArtificialWall(mass_location_, diameter_ * 0.5);
    translation_force_on_point_mass[0] += force_from_artificial_wall[0];
    translation_force_on_point_mass[1] += force_from_artificial_wall[1];
    translation_force_on_point_mass[2] += force_from_artificial_wall[2];
  }

  // 2) Spring force from my neurites (translation and rotation)--------------------------
  for (auto c : daughters_) {
    auto force_from_daughter = c->forceTransmittedFromDaugtherToMother(
        std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
    // for mass translation
    translation_force_on_point_mass[0] += force_from_daughter[0];
    translation_force_on_point_mass[1] += force_from_daughter[1];
    translation_force_on_point_mass[2] += force_from_daughter[2];
    // for rotation
    auto xyz = daughters_coord_[c];
    std::array<double, 3> r { xyz[0] * xAxis[0] + xyz[1] * yAxis[0] + xyz[2] * zAxis[0], xyz[0] * xAxis[1]
        + xyz[1] * yAxis[1] + xyz[2] * zAxis[1], xyz[0] * xAxis[2] + xyz[1] * yAxis[2] + xyz[2] * zAxis[2] };
    rotation_force = Matrix::add(rotation_force, Matrix::crossProduct(r, force_from_daughter));
  }
  // 3) Object avoidance force -----------------------------------------------------------
  //  (We check for every neighbor object if they touch us, i.e. push us away)
  for (auto neighbor : so_node_->getNeighbors()) {
    // of course, only if it is an instance of std::shared_ptr<PhysicalObject>
    if (neighbor->isAPhysicalObject()) {
      auto n = std::static_pointer_cast<PhysicalObject>(neighbor);
      // if it is a direct relative, we don't take it into account
      if (neighbor->isAPhysicalCylinder()
          && STLUtil::listContains(daughters_, std::static_pointer_cast<PhysicalCylinder>(neighbor))) {  // no physical effect of a member of the family...
        continue;
      }
      // if we have a PhysicalBond with him, we also don't take it into account
      for (auto pb : physical_bonds_) {
        if (pb->getOppositePhysicalObject(std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()))
            == neighbor) {
          continue;
        }
      }
      auto force_from_this_neighbor = n->getForceOn(std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
      translation_force_on_point_mass[0] += force_from_this_neighbor[0];
      translation_force_on_point_mass[1] += force_from_this_neighbor[1];
      translation_force_on_point_mass[2] += force_from_this_neighbor[2];
    }
  }

  // 4) PhysicalBonds--------------------------------------------------------------------
  for (auto pb : physical_bonds_) {
    auto force_from_this_physical_bond = pb->getForceOn(
        std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()));
    // for mass translation only (no rotation)
    translation_force_on_point_mass[0] += force_from_this_physical_bond[0];
    translation_force_on_point_mass[1] += force_from_this_physical_bond[1];
    translation_force_on_point_mass[2] += force_from_this_physical_bond[2];
  }
  // How the physics influences the next displacement--------------------------------------------------------
  total_force_last_time_step_[0] = translation_force_on_point_mass[0];  // for Force display in View
  total_force_last_time_step_[1] = translation_force_on_point_mass[1];
  total_force_last_time_step_[2] = translation_force_on_point_mass[2];
  total_force_last_time_step_[3] = -1;   // we don't know yet if it will result in a movement
  //**UNLOCK W
  double norm_of_force = ecm_->sqrt(
      translation_force_on_point_mass[0] * translation_force_on_point_mass[0]
          + translation_force_on_point_mass[1] * translation_force_on_point_mass[1]
          + translation_force_on_point_mass[2] * translation_force_on_point_mass[2]);

  // is there enough force to :
  //  - make us biologically move (Tractor) :
  if (Matrix::norm(tractor_force_) > 0.01) {
    biological_translation = true;
  }
  //  - break adherence and make us translate ?
  if (norm_of_force > adherence_) {
    physical_translation = true;
  }
  //  - make us rotate ?
  double r = Matrix::norm(rotation_force);
  if (r > rotational_inertia_) {
    physical_rotation = true;
  }

  double mh = h / mass_;
  // adding the physics translation (scale by weight) if important enough
  if (physical_translation) {

    // We scale the move with mass and time step
    movement_at_next_step[0] += translation_force_on_point_mass[0] * mh;
    movement_at_next_step[1] += translation_force_on_point_mass[1] * mh;
    movement_at_next_step[2] += translation_force_on_point_mass[2] * mh;

  }

  // Performing the translation itself :
  if (physical_translation || biological_translation) {

    total_force_last_time_step_[3] = 1;  // it does become a movement

    // but we want to avoid huge jumps in the simulation, so there are maximum distances possible
    if (norm_of_force * mh > Param::kSimulationMaximalDisplacement) {
      movement_at_next_step = Matrix::scalarMult(Param::kSimulationMaximalDisplacement,
                                                 Matrix::normalize(movement_at_next_step));
    }

    // The translational movement itself
    mass_location_[0] += movement_at_next_step[0];
    mass_location_[1] += movement_at_next_step[1];
    mass_location_[2] += movement_at_next_step[2];

  }
  // Performing the rotation
  if (physical_rotation) {
    double rotation_angle = 3.14 * Param::kSimulationTimeStep;
    x_axis_ = Matrix::rotAroundAxis(xAxis, rotation_angle, rotation_force, ecm_);
    y_axis_ = Matrix::rotAroundAxis(yAxis, rotation_angle, rotation_force, ecm_);
    z_axis_ = Matrix::rotAroundAxis(zAxis, rotation_angle, rotation_force, ecm_);
  }
  // updating some values :
  if (biological_translation || physical_translation || physical_rotation) {
    // re-centering my SpatialOrganizationNode
    updateSpatialOrganizationNodePosition();
    // if I have daughters, I update their length and tension
    for (auto c : daughters_) {
      c->updateDependentPhysicalVariables();
    }

    // Re-schedule me and every one that has something to do with me :
    setOnTheSchedulerListForPhysicalObjects(true);
    // daughters :
    for (auto c : daughters_) {
      c->setOnTheSchedulerListForPhysicalObjects(true);
    }
    // neighbors :
    for (auto neighbor : so_node_->getNeighbors()) {
      if (neighbor->isAPhysicalObject()) {
        std::static_pointer_cast<PhysicalObject>(neighbor)->setOnTheSchedulerListForPhysicalObjects(true);
      }
    }
    // physical objects at the other side of a PhysicalBond:
    for (auto pb : physical_bonds_) {
      pb->getOppositePhysicalObject(std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()))
          ->setOnTheSchedulerListForPhysicalObjects(true);
    }
  }

  // Reset biological movement to 0.
  // (Will need new instruction from SomaElement in order to move again)
  tractor_force_[0] = 0;
  tractor_force_[1] = 0;
  tractor_force_[2] = 0;
}

std::array<double, 3> PhysicalSphere::getAxis() const {
  return z_axis_;
}

std::list<std::shared_ptr<PhysicalCylinder>> PhysicalSphere::getDaughters() const {
  return daughters_;
}

void PhysicalSphere::runIntracellularDiffusion() {

  // 1) Degradation according to the degradation constant for each chemical
  for (auto el : intracellular_substances_) {
    auto is = el.second;
    is->degrade();
    if (is->isVolumeDependant()) {
      is->updateQuantityBasedOnConcentration(volume_);
    } else {
      is->updateQuantityBasedOnConcentration(getLength());
    }
  }

  //  2) Diffusion in Physical cylinders
  // TODO : scramble daughters so that we don't always go in same order.
  auto daugters = daughters_;  //fixme typo - copy really needed?
  for (auto cyl : daughters_) {
    // To be sure that we diffuse all the chemicals,
    // the direction (i.e.who calls diffuseWithThisstd::shared_ptr<PhysicalObject>() )
    // is chosen randomly.
    auto po1 = std::static_pointer_cast<PhysicalObject>(this->shared_from_this());
    std::shared_ptr<PhysicalObject> po2 = cyl;
    if (ecm_->getRandomDouble1() < 0.5) {
      po1 = cyl;
      po2 = std::static_pointer_cast<PhysicalSphere>(this->shared_from_this());
    }
    // now we call the diffusion function in the super class
    po1->diffuseWithThisPhysicalObjects(po2, cyl->getActualLength());
  }
}

std::array<double, 3> PhysicalSphere::transformCoordinatesGlobalToLocal(const std::array<double, 3>& position) const {
  auto tmp = Matrix::subtract(position, mass_location_);
  return {
    Matrix::dot(tmp, x_axis_),
    Matrix::dot(tmp, y_axis_),
    Matrix::dot(tmp, z_axis_)
  };
}

std::array<double, 3> PhysicalSphere::transformCoordinatesLocalToGlobal(const std::array<double, 3>& pos) const {
  std::array<double, 3> glob { pos[0] * x_axis_[0] + pos[1] * y_axis_[0] + pos[2] * z_axis_[0], pos[0] * x_axis_[1]
      + pos[1] * y_axis_[1] + pos[2] * z_axis_[1], pos[0] * x_axis_[2] + pos[1] * y_axis_[2] + pos[2] * z_axis_[2] };
  return Matrix::add(glob, mass_location_);
}

std::array<double, 3> PhysicalSphere::transformCoordinatesLocalToPolar(const std::array<double, 3>& pos) const {
  return {
    ecm_->sqrt( pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]),
    ecm_->atan2( ecm_->sqrt( pos[0] * pos[0] + pos[1] * pos[1]), pos[2]),
    ecm_->atan2(pos[1], pos[0])
  };
}

std::array<double, 3> PhysicalSphere::transformCoordinatesPolarToLocal(const std::array<double, 3>& pos) const {
  return {
    pos[0] * ecm_->cos(pos[2])* ecm_->sin(pos[1]),
    pos[0] * ecm_->sin(pos[2])* ecm_->sin(pos[1]),
    pos[0] * ecm_->cos(pos[1])
  };
}

std::array<double, 3> PhysicalSphere::transformCoordinatesPolarToGlobal(const std::array<double, 2>& pos) const {

  double localX = pos[0] * ecm_->cos(pos[2]) * ecm_->sin(pos[1]);
  double localY = pos[0] * ecm_->sin(pos[2]) * ecm_->sin(pos[1]);
  double localZ = pos[0] * ecm_->cos(pos[1]);
  return {
    mass_location_[0] + localX * x_axis_[0] + localY * y_axis_[0] + localZ * z_axis_[0],
    mass_location_[1] + localX * x_axis_[1] + localY * y_axis_[1] + localZ * z_axis_[1],
    mass_location_[2] + localX * x_axis_[2] + localY * y_axis_[2] + localZ * z_axis_[2]
  };
}

std::array<double, 3> PhysicalSphere::transformCoordinatesGlobalToPolar(const std::array<double, 3>& pos) const {
  auto vector_to_point = Matrix::subtract(pos, mass_location_);
  std::array<double, 3> local_cartesian { Matrix::dot(x_axis_, vector_to_point), Matrix::dot(y_axis_, vector_to_point),
      Matrix::dot(z_axis_, vector_to_point) };
  return {
    ecm_->sqrt(local_cartesian[0] * local_cartesian[0] + local_cartesian[1] * local_cartesian[1] + local_cartesian[2] * local_cartesian[2]),
    ecm_->atan2( ecm_->sqrt(local_cartesian[0] * local_cartesian[0] + local_cartesian[1] * local_cartesian[1]), local_cartesian[2]),
    ecm_->atan2(local_cartesian[1], local_cartesian[0])
  };
}

std::array<double, 3> PhysicalSphere::getUnitNormalVector(const std::array<double, 3>& position) const {
  auto local = transformCoordinatesPolarToLocal(position);
  return {
    local[0] * x_axis_[0] + local[1] * y_axis_[0] + local[2] * z_axis_[0],
    local[0] * x_axis_[1] + local[1] * y_axis_[1] + local[2] * z_axis_[1],
    local[0] * x_axis_[2] + local[1] * y_axis_[2] + local[2] * z_axis_[2]
  };
}

CellElement* PhysicalSphere::getCellElement() const {
  return getSomaElement();
}

bool PhysicalSphere::isRelative(const std::shared_ptr<PhysicalObject>& po) const {
  return po->isAPhysicalCylinder() && STLUtil::listContains(daughters_, std::static_pointer_cast<PhysicalCylinder>(po));
}

double PhysicalSphere::getLength() const {
  return diameter_;
}

std::array<double, 3> PhysicalSphere::forceTransmittedFromDaugtherToMother(
    const std::shared_ptr<PhysicalObject>& motherWhoAsks) {
  return {0, 0, 0};  //fixme should return null
}

void PhysicalSphere::removeDaugther(const std::shared_ptr<PhysicalObject>& po) {
  if (po->isAPhysicalCylinder()) {
    auto daughter = std::static_pointer_cast<PhysicalCylinder>(po);
    daughters_.remove(daughter);
    daughters_coord_.erase(daughter);
  }
}

void PhysicalSphere::updateRelative(const std::shared_ptr<PhysicalObject>& old_rel,
                                    const std::shared_ptr<PhysicalObject>& new_rel) {
  std::array<double, 3> coord = daughters_coord_[std::static_pointer_cast<PhysicalCylinder>(old_rel)];
  auto old_cyl = std::static_pointer_cast<PhysicalCylinder>(old_rel);
  auto new_cyl = std::static_pointer_cast<PhysicalCylinder>(new_rel);
  daughters_.remove(old_cyl);
  daughters_.push_back(new_cyl);
  daughters_coord_[new_cyl] = coord;
}

void PhysicalSphere::updateDependentPhysicalVariables() {
  updateVolume();
}

void PhysicalSphere::updateIntracellularConcentrations() {
  for (auto el : intracellular_substances_) {
    auto s = el.second;
    if (s->isVolumeDependant()) {
      s->updateConcentrationBasedOnQuantity(volume_);
    } else {
      s->updateConcentrationBasedOnQuantity(diameter_);
    }
  }
}

void PhysicalSphere::updateVolume() {
  volume_ = 0.523598776 * diameter_ * diameter_ * diameter_;  // 0,523598776 = (4/3)*pi*(1/(2^3))
  updateIntracellularConcentrations();
}

void PhysicalSphere::updateDiameter() {
  // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
  diameter_ = ecm_->cbrt(volume_ * 1.90985932);      // 1.90985932 = 6/pi
}

void PhysicalSphere::updateSpatialOrganizationNodePosition() {
  std::array<double, 3> current_center = so_node_->getPosition();
  std::array<double, 3> displacement { mass_location_[0] - current_center[0], mass_location_[1] - current_center[1],
      mass_location_[2] - current_center[2] };
  double offset = Matrix::norm(displacement);
  if (offset > diameter_ * 0.25 || offset > 5) {
    // TODO : do we need this ?
    displacement = Matrix::add(displacement, ecm_->matrixRandomNoise3(diameter_ * 0.025));
    so_node_->moveFrom(displacement);  //fixme catch PositionNotAllowedException
  }
}

void PhysicalSphere::scheduleMeAndAllMyFriends() {

  // Re-schedule me and every one that has something to do with me :
  setOnTheSchedulerListForPhysicalObjects(true);
  // daughters :
  for (auto c : daughters_) {
    c->setOnTheSchedulerListForPhysicalObjects(true);
  }
  // neighbors :
  for (auto neighbor : so_node_->getNeighbors()) {
    if (neighbor->isAPhysicalObject()) {
      std::static_pointer_cast<PhysicalObject>(neighbor)->setOnTheSchedulerListForPhysicalObjects(true);
    }
  }
  // physical objects at the other side of a PhysicalBond:
  for (auto pb : physical_bonds_) {
    pb->getOppositePhysicalObject(std::static_pointer_cast<PhysicalSphere>(this->shared_from_this()))
        ->setOnTheSchedulerListForPhysicalObjects(true);
  }
}

}  // namespace physics
}  // namespace cx3d
