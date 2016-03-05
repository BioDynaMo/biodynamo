#include "physics/substance.h"

#include <sstream>
#include <cmath>

#include "string_util.h"
#include "sim_state_serialization_util.h"

namespace cx3d {
namespace physics {

Substance::Substance()
    : diffusion_constant_ { 1000 },
      degradation_constant_ { 0.01 },
      color_ { 0xFF },  // sets color to blue
      quantity_ { 0.0 },
      concentration_ { 0.0 } {

}

Substance::Substance(const std::string& id, double diffusion_constant, double degradation_constant)
    : id_ { id },
      diffusion_constant_ { diffusion_constant },
      degradation_constant_ { degradation_constant },
      color_ { 0xFF },  // sets color to blue
      quantity_ { 0.0 },
      concentration_ { 0.0 } {

}

Substance::Substance(const std::string& id, Color color)
    : id_ { id },
      diffusion_constant_ { 1000 },
      degradation_constant_ { 0.01 },
      color_ { color },
      quantity_ { 0.0 },
      concentration_ { 0.0 } {

}

Substance::Substance(const Substance& other)
    : id_ { other.id_ },
      diffusion_constant_ { other.diffusion_constant_ },
      degradation_constant_ { other.degradation_constant_ },
      color_ { other.color_ },
      quantity_ { 0.0 },
      concentration_ { 0.0 } {
}

StringBuilder& Substance::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::keyValue(sb, "id", id_, true);
  SimStateSerializationUtil::keyValue(sb, "diffusionConstant", diffusion_constant_);
  SimStateSerializationUtil::keyValue(sb, "degradationConstant", degradation_constant_);
  SimStateSerializationUtil::keyValue(sb, "color", SimStateSerializationUtil::colorToHexString(color_.getValue()),
                                      true);
  SimStateSerializationUtil::keyValue(sb, "quantity", quantity_);
  SimStateSerializationUtil::keyValue(sb, "concentration", concentration_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

void Substance::changeQuantityFrom(double delta_q) {
  quantity_ += delta_q;
  if (quantity_ < 0) {
    quantity_ = 0;
  }
}

void Substance::multiplyQuantityAndConcentrationBy(double factor) {
  quantity_ *= factor;
  setConcentration(concentration_ *= factor);
}

void Substance::updateQuantityBasedOnConcentration(double volume) {
  quantity_ = concentration_ * volume;
}

void Substance::updateConcentrationBasedOnQuantity(double volume) {
  setConcentration(quantity_ / volume);
}

bool Substance::equalTo(const std::shared_ptr<Substance>& o) {

  return id_.compare(o->id_) == 0 && color_ == o->color_
      && std::abs(degradation_constant_ - o->degradation_constant_) < 1E-10
      && std::abs(diffusion_constant_ - o->diffusion_constant_) < 1E-10;
}

Color Substance::getConcentrationDependentColor() const {  // todo bad design - view code shouldn't be here
  // only used by graphics package -> not ported
//  int alpha = (int) (255.0 * concentration_ * View.chemicalDrawFactor);
//  if (alpha < 0) {
//    alpha = 0;
//  } else if (alpha > 255) {
//    alpha = 255;
//  }
//  return Color((color_.getValue() & 0xFF000000) | (alpha << 24));
}

std::string Substance::getId() const {
  return id_;
}

void Substance::setId(const std::string& id) {
  id_ = id;
}

double Substance::getDiffusionConstant() const {
  return diffusion_constant_;
}

void Substance::setDiffusionConstant(double diffusion_constant) {
  diffusion_constant_ = diffusion_constant;
}

double Substance::getDegradationConstant() const {
  return degradation_constant_;
}

void Substance::setDegradationConstant(double degradation_constant) {
  degradation_constant_ = degradation_constant;
}

Color Substance::getColor() const {
  return color_;
}

void Substance::setColor(Color color) {
  color_ = color;
}

double Substance::getConcentration() const {
  return concentration_;
}

void Substance::setConcentration(double concentration) {
  if (concentration < 0.0) {
    concentration = 0.0;  //todo possible bug in java version
  } else {
    concentration_ = concentration;
  }
}

double Substance::getQuantity() const {
  return quantity_;
}

void Substance::setQuantity(double quantity) {
  quantity_ = quantity;
}

std::shared_ptr<Substance> Substance::getCopy() const {
  return std::shared_ptr<Substance>(new Substance(*this));
}

std::string Substance::toString() const {
  return "SubstanceID" + id_;
//  std::ostringstream str_stream;
//  str_stream << "(";
//  str_stream << StringUtil::toStr(id_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(quantity_);
//  str_stream << ", ";
//  str_stream << StringUtil::toStr(concentration_);
//  str_stream << ")";
//  return str_stream.str();
}

}  // namespace physics
}  // namespace cx3d
