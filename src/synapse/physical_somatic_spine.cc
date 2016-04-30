#include "synapse/physical_somatic_spine.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "synapse/biological_somatic_spine.h"
#include "physics/physical_object.h"

namespace cx3d {
namespace synapse {

std::shared_ptr<PhysicalSomaticSpine> PhysicalSomaticSpine::create() {
  return std::shared_ptr<PhysicalSomaticSpine>(new PhysicalSomaticSpine());
}

std::shared_ptr<PhysicalSomaticSpine> PhysicalSomaticSpine::create(const std::shared_ptr<physics::PhysicalObject>& po,
                                                     const std::array<double, 2>& origin, double length) {
  return std::shared_ptr<PhysicalSomaticSpine>(new PhysicalSomaticSpine(po, origin, length));
}

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

  SimStateSerializationUtil::keyValue(sb, "biologicalSomaticSpine", biological_spine_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

bool PhysicalSomaticSpine::synapseWith(const std::shared_ptr<Excrescence>& other_excrescence, bool create_physical_bond) {
  // only if the other Excrescence is a bouton
  if (other_excrescence->getType() != Excrescence::Type::kBouton) {
    // todo throw exception?
    return false;
  }
  // making the references
  ex_ = other_excrescence;
  ex_->setEx(shared_from_this());
  // if needed, formation of the PhysicalBound
  if (create_physical_bond) {
    auto global_pos_on_po = po_->transformCoordinatesPolarToGlobal(position_on_po_);
    auto ex_global_pos_on_po = ex_->getPo()->transformCoordinatesPolarToGlobal(ex_->getPositionOnPO());
    auto distance = Matrix::distance(global_pos_on_po, ex_global_pos_on_po);
    auto pb = ecm_->newPhysicalBond(po_, position_on_po_, ex_->getPo(), ex_->getPositionOnPO(), distance, 1);
  }
  return true;
}

bool PhysicalSomaticSpine::synapseWithSoma(const std::shared_ptr<Excrescence>& other_excrescence, bool create_phyiscal_bond) {
  return false;
}

bool PhysicalSomaticSpine::synapseWithShaft(const std::shared_ptr<local_biology::NeuriteElement>& other_ne, double max_dis,
                                     int nr_segments, bool create_phyiscal_bond) {
  return false;
}

std::shared_ptr<BiologicalSomaticSpine> PhysicalSomaticSpine::getBiologicalSomaticSpine() const {
  return biological_spine_;
}

void PhysicalSomaticSpine::setBiologicalSomaticSpine(const std::shared_ptr<BiologicalSomaticSpine>& spine) {
  biological_spine_ = spine;
}

}  // namespace synapse
}  // namespace cx3d
