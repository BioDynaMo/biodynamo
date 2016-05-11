#include "synapse/excrescence.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"

#include "local_biology/neurite_element.h"
#include "physics/physical_object.h"

namespace cx3d {
namespace synapse {

std::shared_ptr<simulation::ECM> Excrescence::ecm_ { nullptr };

Excrescence::Excrescence() {
}

Excrescence::Excrescence(Excrescence::Type type)
    : type_ { type } {
}

Excrescence::Excrescence(const std::shared_ptr<physics::PhysicalObject> po, const std::array<double, 2>& position_on_po,
                         double length, Excrescence::Type type)
    : po_ { po },
      position_on_po_ ( position_on_po ),
      length_ { length },
      type_ { type } {

}

Excrescence::~Excrescence() {
}

StringBuilder& Excrescence::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  //po is circular reference
  SimStateSerializationUtil::keyValue(sb, "ex", ex_);
  SimStateSerializationUtil::keyValue(sb, "positionOnPO", position_on_po_);
  SimStateSerializationUtil::keyValue(sb, "length", length_);
  SimStateSerializationUtil::keyValue(sb, "type", type_);

  return sb;
}

bool Excrescence::equalTo(const std::shared_ptr<Excrescence>& other) const {
  return this == other.get();
}

std::array<double, 3> Excrescence::getProximalEnd() const {
  return po_->transformCoordinatesPolarToGlobal(position_on_po_);
}

std::array<double, 3> Excrescence::getDistalEnd() const {
  auto prox = po_->transformCoordinatesPolarToGlobal(position_on_po_);
  // if no synapse, defined by the length
  // if a synapse is made, this is the middle of the distance between the two attachment points
  if (ex_ == nullptr) {
    auto axis = po_->getUnitNormalVector( { position_on_po_[0], position_on_po_[1], 0.0 });
    return Matrix::add(prox, Matrix::scalarMult(length_, axis));
  } else {
    // if a synapse is made, this is the middle of the distance between the two attachment points
    return Matrix::scalarMult(0.5, Matrix::add(getProximalEnd(), ex_->getProximalEnd()));
  }
}

std::shared_ptr<Excrescence> Excrescence::getEx() const {
  return ex_;
}

void Excrescence::setEx(const std::shared_ptr<Excrescence>& e) {
  ex_ = e;
}

double Excrescence::getLength() const {
  return length_;
}

void Excrescence::setLength(double length) {
  length_ = length;
}

std::shared_ptr<physics::PhysicalObject> Excrescence::getPo() const {
  return po_;
}

void Excrescence::setPo(const std::shared_ptr<physics::PhysicalObject>& po) {
  po_ = po;
}

std::array<double, 2> Excrescence::getPositionOnPO() const {
  return position_on_po_;
}

void Excrescence::setPositionOnPO(const std::array<double, 2>& position) {
  position_on_po_ = position;
}

int Excrescence::getType() const {
  return type_;
}

void Excrescence::setType(int type) {
  type_ = type;
}

}  // namespace synapse
}  // namespace cx3d
