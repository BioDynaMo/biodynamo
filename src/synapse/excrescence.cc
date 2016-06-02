#include "synapse/excrescence.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"

namespace bdm {
namespace synapse {

ECM* Excrescence::ecm_ = ECM::getInstance();

Excrescence::Excrescence() {
}

Excrescence::Excrescence(Excrescence::Type type)
    : type_ { type } {
}

Excrescence::Excrescence(PhysicalObject* po, const std::array<double, 2>& position_on_po, double length,
                         Excrescence::Type type)
    : po_ { po },
      position_on_po_(position_on_po),
      length_ { length },
      type_ { type } {

}

Excrescence::~Excrescence() {
}

StringBuilder& Excrescence::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::keyValue(sb, "ex", ex_);
  SimStateSerializationUtil::keyValue(sb, "positionOnPO", position_on_po_);
  SimStateSerializationUtil::keyValue(sb, "length", length_);
  SimStateSerializationUtil::keyValue(sb, "type", type_);

  return sb;
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

Excrescence* Excrescence::getEx() const {
  return ex_;
}

void Excrescence::setEx(Excrescence* e) {
  ex_ = e;
}

double Excrescence::getLength() const {
  return length_;
}

void Excrescence::setLength(double length) {
  length_ = length;
}

PhysicalObject* Excrescence::getPo() const {
  return po_;
}

void Excrescence::setPo(PhysicalObject* po) {
  po_ = po;
}

std::array<double, 2> Excrescence::getPositionOnPO() const {
  return position_on_po_;
}

void Excrescence::setPositionOnPO(const std::array<double, 2>& position) {
  position_on_po_ = position;
}

Excrescence::Type Excrescence::getType() const {
  return type_;
}

void Excrescence::setType(Type type) {
  type_ = type;
}

}  // namespace synapse
}  // namespace bdm
