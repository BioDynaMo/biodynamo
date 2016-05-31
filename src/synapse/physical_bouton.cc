#include "synapse/physical_bouton.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "synapse/biological_bouton.h"
#include "local_biology/neurite_element.h"
#include "physics/physical_object.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"

namespace cx3d {
namespace synapse {

using physics::PhysicalBond;

PhysicalBouton::PhysicalBouton()
    : Excrescence(Excrescence::Type::kBouton) {
}

PhysicalBouton::PhysicalBouton(PhysicalObject* po, const std::array<double, 2>& origin,
                               double length)
    : Excrescence(po, origin, length, Excrescence::Type::kBouton) {
}

PhysicalBouton::~PhysicalBouton() {
}

StringBuilder& PhysicalBouton::simStateToJson(StringBuilder& sb) const {
  Excrescence::simStateToJson(sb);

  SimStateSerializationUtil::keyValue(sb, "biologicalBouton", biological_bouton_.get());

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

bool PhysicalBouton::synapseWith(Excrescence* other_excrescence, bool create_phyiscal_bond) {
  // only if the other Excrescence is a bouton
  if (other_excrescence->getType() != Excrescence::Type::kSpine) {
    return false;
  }
  // no autapses
  if (po_->getCellElement() != nullptr && other_excrescence->getPo()->getCellElement() != nullptr) {
    if (po_->getCellElement()->getCell() == other_excrescence->getPo()->getCellElement()->getCell()) {
      return false;
    }
  }
  // making the references
  ex_ = other_excrescence;
  ex_->setEx(this);
  // if needed, formation of the PhysicalBound
  if (create_phyiscal_bond) {
    auto global_pos_on_po = po_->transformCoordinatesPolarToGlobal(position_on_po_);
    auto ex_global_pos_on_po = ex_->getPo()->transformCoordinatesPolarToGlobal(ex_->getPositionOnPO());
    auto distance = Matrix::distance(global_pos_on_po, ex_global_pos_on_po);
    auto pb = PhysicalBond::create(po_, position_on_po_, ex_->getPo(), ex_->getPositionOnPO(), distance, 1);
  }

  return true;
}

bool PhysicalBouton::synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) {
  // only if the other Excrescence is a bouton
  if (other_excrescence->getType() == Excrescence::Type::kBouton) {
    return false;
  }
  // making the references
  ex_ = other_excrescence;
  ex_->setEx(this);
  // if needed, formation of the PhysicalBound
  if (create_phyiscal_bond) {
    auto global_pos_on_po = po_->transformCoordinatesPolarToGlobal(position_on_po_);
    auto ex_global_pos_on_po = ex_->getPo()->transformCoordinatesPolarToGlobal(ex_->getPositionOnPO());
    auto distance = Matrix::distance(global_pos_on_po, ex_global_pos_on_po);
    auto pb = PhysicalBond::create(po_, position_on_po_, ex_->getPo(), ex_->getPositionOnPO(), distance, 1);
  }
  return true;
}

bool PhysicalBouton::synapseWithShaft(NeuriteElement* other_ne, double max_dis,
                                      int nr_segments, bool create_phyiscal_bond) {
  auto pc = other_ne->getPhysicalCylinder();
  double neLength = pc->getActualLength();
  std::array<double, 3> currPos;
  std::array<double, 3> currVec;

  auto currDir = pc->getUnitaryAxisDirectionVector();

  double dX = neLength * 0.5;
  currVec = {-currDir[0] * dX, -currDir[1] * dX,
    -currDir[2] * dX};

  currPos = other_ne->getPhysicalCylinder()->transformCoordinatesLocalToGlobal(currVec);
  if ((Matrix::norm(Matrix::subtract(currPos, getProximalEnd())) < max_dis) && create_phyiscal_bond) {

    auto tmp = other_ne->getPhysicalCylinder()->transformCoordinatesLocalToPolar(currVec);
    double distance = Matrix::distance(getProximalEnd(), currPos);
    auto other_position_on_po = std::array<double, 2> { tmp[0], tmp[1] };
    auto pb = PhysicalBond::create(po_, position_on_po_, other_ne->getPhysicalCylinder(), other_position_on_po,
                                    distance, 1);
    ne_shaft_ = other_ne;

  }
  return true;
}

void PhysicalBouton::setBiologicalBouton(BiologicalBouton::UPtr bouton) {
  biological_bouton_ = std::move(bouton);
}

BiologicalBouton* PhysicalBouton::getBiologicalBouton() const {
  return biological_bouton_.get();
}

}  // namespace synapse
}  // namespace cx3d
