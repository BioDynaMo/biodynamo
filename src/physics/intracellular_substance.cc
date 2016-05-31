#include "physics/intracellular_substance.h"

#include "sim_state_serialization_util.h"

namespace cx3d {
namespace physics {

IntracellularSubstance::IntracellularSubstance()
    : Substance(),
      asymmetry_constant_ { 0.0 },
      visible_from_outside_ { false },
      volume_dependant_ { false } {

}

IntracellularSubstance::IntracellularSubstance(const std::string& id, double diffusion_constant,
                                               double degradation_constant)
    : Substance(id, diffusion_constant, degradation_constant),
      asymmetry_constant_ { 0.0 },
      visible_from_outside_ { false },
      volume_dependant_ { false } {

}

IntracellularSubstance::IntracellularSubstance(const IntracellularSubstance& other)
    : Substance(other),
      asymmetry_constant_ { other.asymmetry_constant_ },
      visible_from_outside_ { other.visible_from_outside_ },
      volume_dependant_ { other.volume_dependant_ } {
}

StringBuilder& IntracellularSubstance::simStateToJson(StringBuilder& sb) const {
  Substance::simStateToJson(sb);
  SimStateSerializationUtil::removeLastChar(sb);
  sb.append(",");

  SimStateSerializationUtil::keyValue(sb, "visibleFromOutside", visible_from_outside_);
  SimStateSerializationUtil::keyValue(sb, "volumeDependant", volume_dependant_);
  SimStateSerializationUtil::keyValue(sb, "asymmetryConstant", asymmetry_constant_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

void IntracellularSubstance::distributeConcentrationOnDivision(IntracellularSubstance* new_is) {
  double p = asymmetry_constant_ * (1 - concentration_) * concentration_;
  new_is->setConcentration(concentration_ * (1 + p));
  concentration_ = concentration_ * (1 - p);
}

void IntracellularSubstance::degrade() {
  concentration_ = concentration_ * (1 - degradation_constant_ * Param::kSimulationTimeStep);
}

double IntracellularSubstance::getAsymmetryConstant() const {
  return asymmetry_constant_;
}

void IntracellularSubstance::setAsymmetryConstant(double asymmetry_constant) {
  asymmetry_constant_ = asymmetry_constant;
}

bool IntracellularSubstance::isVisibleFromOutside() const {
  return visible_from_outside_;
}

void IntracellularSubstance::setVisibleFromOutside(bool visible_from_outside) {
  visible_from_outside_ = visible_from_outside;
}

bool IntracellularSubstance::isVolumeDependant() const {
  return volume_dependant_;
}

void IntracellularSubstance::setVolumeDependant(bool volume_dependant) {
  volume_dependant_ = volume_dependant;
}

Substance::UPtr IntracellularSubstance::getCopy() const {
  return Substance::UPtr(new IntracellularSubstance(*this));
}

}  // namespace physics
}  // namespace cx3d
