#include "local_biology/neurite_element.h"

#include <iostream>
#include <cmath>

#include "matrix.h"
#include "sim_state_serialization_util.h"

#include "local_biology/local_biology_module.h"

#include "spatial_organization/space_node.h"

#include "synapse/biological_spine.h"
#include "synapse/biological_bouton.h"
#include "synapse/physical_spine.h"
#include "synapse/physical_bouton.h"

#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"

namespace cx3d {
namespace local_biology {

using physics::PhysicalObject;
using namespace cx3d::synapse;

NeuriteElement::NeuriteElement()
    : CellElement() {
  ecm_->addNeuriteElement(this);
}

NeuriteElement::~NeuriteElement() {
  ecm_->removeNeuriteElement(this);
}

StringBuilder& NeuriteElement::simStateToJson(StringBuilder& sb) const {
  CellElement::simStateToJson(sb);

  SimStateSerializationUtil::keyValue(sb, "physicalCylinder", physical_cylinder_.get());
  SimStateSerializationUtil::keyValue(sb, "isAnAxon", is_axon_);
  SimStateSerializationUtil::removeLastChar(sb);

  sb.append("}");
  return sb;
}

NeuriteElement* NeuriteElement::getCopy() const {
  auto ne = UPtr { new NeuriteElement() };
  ne->is_axon_ = is_axon_;
  ne->cell_ = cell_;
  auto ne_raw = ne.get();
  cell_->addNeuriteElement(std::move(ne));
  return ne_raw;
}

void NeuriteElement::removeYourself() {
  // todo check if still necessary - see destructor
  ecm_->removeNeuriteElement(this);
}

void NeuriteElement::run() {
  runLocalBiologyModules();
}

void NeuriteElement::retractTerminalEnd(double speed) {
  physical_cylinder_->retractCylinder(speed);
}

void NeuriteElement::elongateTerminalEnd(double speed, const std::array<double, 3>& direction) {
  physical_cylinder_->extendCylinder(speed, direction);
}

NeuriteElement* NeuriteElement::branch(double diameter, const std::array<double, 3>& direction) {
  // create a new NeuriteElement for side branch
  auto ne = getCopy();
  // todo if direction changed to pointer - define direction if equal to nullptr
  // growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));

  // making the branching at physicalObject level
  auto pc_1 = physical_cylinder_->branchCylinder(1.0, direction);
  // specifying the diameter we wanted
  pc_1->setDiameter(diameter);
  //
  pc_1->setBranchOrder(physical_cylinder_->getBranchOrder() + 1);
  // TODO : Caution : doesn't change the value distally on the main branch

  // linking biology and phyics
  ne->setPhysical(std::move(pc_1));  // (this also sets the call back)

  // Copy of the local biological modules:
  for (auto m : getLocalBiologyModulesList()) {
    if (m->isCopiedWhenNeuriteBranches()) {
      ne->addLocalBiologyModule(m->getCopy());
    }
  }
  return ne;
}

NeuriteElement* NeuriteElement::branch(const std::array<double, 3>& direction) {
  return branch(physical_cylinder_->getDiameter(), direction);
}

NeuriteElement* NeuriteElement::branch(double diameter) {
  auto rand_noise = Random::nextNoise(0.1);
  auto growth_direction = Matrix::perp3(Matrix::add(physical_cylinder_->getUnitaryAxisDirectionVector(), rand_noise),
                                        Random::nextDouble(), ecm_);
  growth_direction = Matrix::normalize(growth_direction);
  return branch(diameter, growth_direction);
}

NeuriteElement* NeuriteElement::branch() {
  double branch_diameter = physical_cylinder_->getDiameter();
  auto rand_noise = Random::nextNoise(0.1);
  auto growth_direction = Matrix::perp3(Matrix::add(physical_cylinder_->getUnitaryAxisDirectionVector(), rand_noise),
                                        Random::nextDouble(), ecm_);
  return branch(branch_diameter, growth_direction);
}

bool NeuriteElement::bifurcationPermitted() const {
  return physical_cylinder_->bifurcationPermitted();
}

std::array<NeuriteElement*, 2> NeuriteElement::bifurcate(double diameter_1, double diameter_2,
                                                         const std::array<double, 3>& direction_1,
                                                         const std::array<double, 3>& direction_2) {
  return bifurcate(Param::kNeuriteDefaultActualLength, diameter_1, diameter_2, direction_1, direction_2);
}

std::array<NeuriteElement*, 2> NeuriteElement::bifurcate(double length, double diameter_1, double diameter_2,
                                                         const std::array<double, 3>& direction_1,
                                                         const std::array<double, 3>& direction_2) {
  // 1) physical bifurcation
  auto pc = physical_cylinder_->bifurcateCylinder(length, direction_1, direction_2);
  // if bifurcation is not allowed...
  // todo return null if pc == nullptr

  // 2) creating the first daughter branch
  auto ne_1 = getCopy();
  auto pc_1 = pc[0].get();
  ne_1->setPhysical(std::move(pc[0]));
  pc_1->setDiameter(diameter_1);
  pc_1->setBranchOrder(physical_cylinder_->getBranchOrder() + 1);

  // 3) the second one
  auto ne_2 = getCopy();
  auto pc_2 = pc[1].get();
  ne_2->setPhysical(std::move(pc[1]));
  pc_2->setDiameter(diameter_2);
  pc_2->setBranchOrder(physical_cylinder_->getBranchOrder() + 1);

  // 4) the local biological modules :
  for (auto m : local_biology_modules_) {
    // copy...
    if (m->isCopiedWhenNeuriteBranches()) {
      // ...for the first neurite
      ne_1->addLocalBiologyModule(m->getCopy());
      // ...for the second neurite
      ne_2->addLocalBiologyModule(m->getCopy());
    }
    // and remove
    if (m->isDeletedAfterNeuriteHasBifurcated()) {
      removeLocalBiologyModule(m);
    }
  }
  return {ne_1, ne_2};
}

std::array<NeuriteElement*, 2> NeuriteElement::bifurcate(const std::array<double, 3>& direction_1,
                                                         const std::array<double, 3>& direction_2) {
  // initial default length :
  double l = Param::kNeuriteDefaultActualLength;
  // diameters :
  double d = physical_cylinder_->getDiameter();

  return bifurcate(l, d, d, direction_1, direction_2);
}

std::array<NeuriteElement*, 2> NeuriteElement::bifurcate() {
  // initial default length :
  double l = Param::kNeuriteDefaultActualLength;
  // diameters :
  double d = physical_cylinder_->getDiameter();
  // direction : (60 degrees between branches)
  double rand = Random::nextDouble();
  auto perp_plane = Matrix::perp3(physical_cylinder_->getSpringAxis(), rand, ecm_);
  double angle_between_branches = Param::kPi / 3.0;
  auto direction_1 = Matrix::rotAroundAxis(physical_cylinder_->getSpringAxis(), angle_between_branches * 0.5,
                                           perp_plane, ecm_);
  auto direction_2 = Matrix::rotAroundAxis(physical_cylinder_->getSpringAxis(), -angle_between_branches * 0.5,
                                           perp_plane, ecm_);

  return bifurcate(l, d, d, direction_1, direction_2);
}

void NeuriteElement::makeSpines(double interval) {
  // how many spines for this NeuriteElement ?
  double length = physical_cylinder_->getActualLength();
  double spine_on_this_segment = length / interval;
  long nb = std::lround(spine_on_this_segment);  //fixme critical
  // TODO : better way to define number (ex : if interval >> length -> no spine at all)
  for (auto i = 0; i < nb; i++) {
    // create the physical part
    std::array<double, 2> coord = { length * Random::nextDouble(), 6.28 * Random::nextDouble() };
    auto p_spine = PhysicalSpine::UPtr { new PhysicalSpine(physical_cylinder_.get(), coord, 3.0) };
    // create the biological part and set call backs
    auto b_spine = BiologicalSpine::UPtr { new BiologicalSpine() };
    b_spine->setPhysicalSpine(p_spine.get());
    p_spine->setBiologicalSpine(std::move(b_spine));
    physical_cylinder_->addExcrescence(std::move(p_spine));
  }
}

void NeuriteElement::makeSingleSpine() {
  double length = physical_cylinder_->getActualLength();
  // create the physical part
  std::array<double, 2> coord = { length * Random::nextDouble(), 6.28 * Random::nextDouble() };
  auto p_spine = PhysicalSpine::UPtr { new PhysicalSpine(physical_cylinder_.get(), coord, 3.0) };
  // create the biological part and set call backs
  auto b_spine = BiologicalSpine::UPtr { new BiologicalSpine() };
  b_spine->setPhysicalSpine(p_spine.get());
  p_spine->setBiologicalSpine(std::move(b_spine));
  physical_cylinder_->addExcrescence(std::move(p_spine));
}

void NeuriteElement::makeSingleSpine(double dist_from_proximal_end) {
  double length = physical_cylinder_->getActualLength();
  if (dist_from_proximal_end > length) {
    std::cout << "NeuriteElement.makeSingleSpine(): no spine formed 'cause this cylinder is shorter than "
        << dist_from_proximal_end << " microns." << std::endl;
    return;
  }
  // create the physical part
  std::array<double, 2> coord = { dist_from_proximal_end, 6.28 * Random::nextDouble() };
  auto p_spine = PhysicalSpine::UPtr { new PhysicalSpine(physical_cylinder_.get(), coord, 3.0) };
  // create the biological part and set call backs
  auto b_spine = BiologicalSpine::UPtr { new BiologicalSpine() };
  b_spine->setPhysicalSpine(p_spine.get());
  p_spine->setBiologicalSpine(std::move(b_spine));
  physical_cylinder_->addExcrescence(std::move(p_spine));
}

void NeuriteElement::makeBoutons(double interval) {
  // how many boutons for this NeuriteElement ?
  double length = physical_cylinder_->getActualLength();
  double boutons_on_this_segment = length / interval;
  long nb = std::lround(boutons_on_this_segment);
  // TODO : better way to define number (ex : if interval >> length -> no spine at all)
  for (int i = 0; i < nb; i++) {
    // create the physical part
    std::array<double, 2> coord = { length * Random::nextDouble(), -3.14 + 6.28 * Random::nextDouble() };
    auto p_bouton = new PhysicalBouton(physical_cylinder_.get(), coord, 2);
    physical_cylinder_->addExcrescence(PhysicalBouton::UPtr { p_bouton });
    // create the biological part and set call backs
    auto b_bouton = BiologicalBouton::UPtr { new BiologicalBouton() };
    b_bouton->setPhysicalBouton(p_bouton);
    p_bouton->setBiologicalBouton(std::move(b_bouton));
  }
}

void NeuriteElement::makeSingleBouton(double dist_from_proximal_end) {
  double length = physical_cylinder_->getActualLength();
  if (dist_from_proximal_end > length) {
    std::cout << "NeuriteElement.makeSingleBouton(): no spine formed 'cause this cylinder is shorter than "
        << dist_from_proximal_end << " microns." << std::endl;
    return;
  }
  // create the physical part
  std::array<double, 2> coord = { dist_from_proximal_end, 6.28 * Random::nextDouble() };
  auto p_bouton = PhysicalBouton::UPtr { new PhysicalBouton(physical_cylinder_.get(), coord, 2) };
  // create the biological part and set call backs
  auto b_bouton = BiologicalBouton::UPtr { new BiologicalBouton() };
  b_bouton->setPhysicalBouton(p_bouton.get());
  p_bouton->setBiologicalBouton(std::move(b_bouton));
  physical_cylinder_->addExcrescence(std::move(p_bouton));
}

void NeuriteElement::makeSingleBouton() {
  // how many boutons for this NeuriteElement ?
  double length = physical_cylinder_->getActualLength();
  // create the physical part
  std::array<double, 2> coord = { length * Random::nextDouble(), -3.14 + 6.28 * Random::nextDouble() };
  auto p_bouton = PhysicalBouton::UPtr { new PhysicalBouton(physical_cylinder_.get(), coord, 2) };
  // create the biological part and set call backs
  auto b_bouton = BiologicalBouton::UPtr { new BiologicalBouton() };
  b_bouton->setPhysicalBouton(p_bouton.get());
  p_bouton->setBiologicalBouton(std::move(b_bouton));
  physical_cylinder_->addExcrescence(std::move(p_bouton));
}

int NeuriteElement::synapseBetweenExistingBS(double probability_to_synapse) {
  int synapses_made = 0;

  auto neighbors = physical_cylinder_->getSoNode()->getNeighbors();
  for (auto pn : neighbors) {
    // For all PhysicalObjects around
    if (!pn->isAPhysicalObject()) {
      continue;
    }
    // with a certain probability
    if (Random::nextDouble() > probability_to_synapse) {
      continue;
    }

    auto po = static_cast<PhysicalObject*>(pn);
    // for all Excrescence pair :
    for (auto e1 : physical_cylinder_->getExcrescences()) {
      // only if this one is a free bouton:
      if (e1->getEx() != nullptr || e1->getType() != Excrescence::Type::kBouton) {
        goto continue_outer;
      }
      for (auto e2 : po->getExcrescences()) {
        // only if the other is a free spine:
        if (e2->getEx() != nullptr || e2->getType() != Excrescence::Type::kSpine) {
          continue;
        }

        // Find origin of the two Excrescences
        auto o1 = e1->getProximalEnd();
        auto o2 = e2->getProximalEnd();
        // vector from o1 to o2
        auto oo = Matrix::subtract(o2, o1);
        // synapse possible only if close enough
        double distoo = Matrix::norm(oo);
        double tol = 0;
        if (distoo > e1->getLength() + e2->getLength() + tol)
          continue;
        // synapse only possible if these two excresscences are pointing toward each other
        oo = Matrix::scalarMult(1 / distoo, oo);  // normalize oo
        auto e1_pos = e1->getPositionOnPO();
        auto e2_pos = e2->getPositionOnPO();
        if (Matrix::dot(oo, physical_cylinder_->getUnitNormalVector( { e1_pos[0], e1_pos[1], 0.0 })) > 0
            && Matrix::dot(oo, po->getUnitNormalVector( { e2_pos[0], e2_pos[1], 0.0 })) < 0) {
          e1->synapseWith(e2, true);
          synapses_made++;
          goto continue_outer;
          // if we made it, now we test the next one
        }
      }
      continue_outer: ;
    }

  }
  return synapses_made;
  // TODO : outer most loop should be e1 (if no excresscence, no check)
  // and : calculation of physical.getUnitNormalVector outside inner most loop
}

PhysicalObject* NeuriteElement::getPhysical() const {
  return physical_cylinder_.get();
}

void NeuriteElement::setPhysical(PhysicalObject::UPtr po) {
  physical_cylinder_ = STLUtil::staticUPtrCast < PhysicalCylinder > (std::move(po));
  physical_cylinder_->setNeuriteElement(this);
}

PhysicalCylinder* NeuriteElement::getPhysicalCylinder() const {
  return physical_cylinder_.get();
}

void NeuriteElement::setPhysicalCylinder(PhysicalCylinder::UPtr pc) {
  physical_cylinder_ = std::move(pc);
  physical_cylinder_->setNeuriteElement(this);
}

bool NeuriteElement::isAxon() const {
  return is_axon_;
}

void NeuriteElement::setAxon(bool is_axon) {
  is_axon_ = is_axon;
}

bool NeuriteElement::isANeuriteElement() const {
  return true;
}

bool NeuriteElement::isASomaElement() const {
  return false;
}

NeuriteElement* NeuriteElement::getDaughterLeft() const {
  if (physical_cylinder_->getDaughterLeft() == nullptr) {
    return nullptr;
  } else {
    return physical_cylinder_->getDaughterLeft()->getNeuriteElement();
  }
}

NeuriteElement* NeuriteElement::getDaughterRight() const {
  if (physical_cylinder_->getDaughterRight() == nullptr) {
    return nullptr;
  } else {
    return physical_cylinder_->getDaughterRight()->getNeuriteElement();
  }
}

std::list<NeuriteElement*> NeuriteElement::addYourselfAndDistalNeuriteElements(std::list<NeuriteElement*>& elements) {
  elements.push_back(this);
  auto left = getDaughterLeft();
  if (left != nullptr) {
    left->addYourselfAndDistalNeuriteElements(elements);
    auto right = getDaughterRight();
    if (right != nullptr) {
      right->addYourselfAndDistalNeuriteElements(elements);
    }
  }
  return elements;
}

}  // namespace local_biology
}  // namespace cx3d
