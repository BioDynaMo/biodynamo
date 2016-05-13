#include "synapse/physical_somatic_spine.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "synapse/biological_somatic_spine.h"
#include "physics/physical_object.h"

namespace cx3d {
namespace synapse {

PhysicalSomaticSpine::PhysicalSomaticSpine()
    : Excrescence(Excrescence::Type::kSomaticSpine) {
}

PhysicalSomaticSpine::PhysicalSomaticSpine(const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin,
                             double length)
    : Excrescence(po, origin, length, Excrescence::Type::kSomaticSpine) {
}

PhysicalSomaticSpine::~PhysicalSomaticSpine() {
}

StringBuilder& PhysicalSomaticSpine::simStateToJson(StringBuilder& sb) const {
  sb.append("{");  //fixme bug: should call Excrescence::simStateToJson

  SimStateSerializationUtil::keyValue(sb, "biologicalSomaticSpine", biological_spine_.get());

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

bool PhysicalSomaticSpine::synapseWith(Excrescence* other_excrescence, bool create_physical_bond) {
  // only if the other Excrescence is a bouton
  if (other_excrescence->getType() != Excrescence::Type::kBouton) {
    // todo throw exception?
    return false;
  }
  // making the references
  ex_ = other_excrescence;
  ex_->setEx(this);
  // if needed, formation of the PhysicalBound
  if (create_physical_bond) {
    auto global_pos_on_po = po_->transformCoordinatesPolarToGlobal(position_on_po_);
    auto ex_global_pos_on_po = ex_->getPo()->transformCoordinatesPolarToGlobal(ex_->getPositionOnPO());
    auto distance = Matrix::distance(global_pos_on_po, ex_global_pos_on_po);
    auto pb = ecm_->newPhysicalBond(po_, position_on_po_, ex_->getPo(), ex_->getPositionOnPO(), distance, 1);
  }
  return true;
}

bool PhysicalSomaticSpine::synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) {
  return false;
}

bool PhysicalSomaticSpine::synapseWithShaft(NeuriteElement* other_ne, double max_dis,
                                     int nr_segments, bool create_phyiscal_bond) {
  return false;
}

BiologicalSomaticSpine* PhysicalSomaticSpine::getBiologicalSomaticSpine() const {
  return biological_spine_.get();
}

void PhysicalSomaticSpine::setBiologicalSomaticSpine(BiologicalSomaticSpine::UPtr spine) {
  biological_spine_ = std::move(spine);
}

}  // namespace synapse
}  // namespace cx3d
