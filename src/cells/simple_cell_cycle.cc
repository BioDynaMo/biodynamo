#include "cells/simple_cell_cycle.h"

#include "cells/cell.h"
#include "sim_state_serialization_util.h"
#include "local_biology/soma_element.h"
#include "physics/physical_sphere.h"

namespace cx3d {
namespace cells {

SimpleCellCycle::SimpleCellCycle() {
}

SimpleCellCycle::~SimpleCellCycle() {
}

StringBuilder& SimpleCellCycle::simStateToJson(StringBuilder& sb) const {
  AbstractCellModule::simStateToJson(sb);

  SimStateSerializationUtil::keyValue(sb, "enable", enabled_);
  SimStateSerializationUtil::keyValue(sb, "dVdt", dVdt_);
  SimStateSerializationUtil::keyValue(sb, "minimumDiameter", minimum_diameter_);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

void SimpleCellCycle::run() {
  if (!enabled_) {
    return;
  }
  auto ps = cell_->getSomaElement()->getPhysicalSphere();
  // is diameter smaller than min
  if (ps->getDiameter() < minimum_diameter_) {
    ps->changeVolume(dVdt_);
  } else {
    // otherwise divide
    cell_->divide();
  }
}

std::shared_ptr<CellModule> SimpleCellCycle::getCopy() const {
  auto cc = std::shared_ptr<SimpleCellCycle>(new SimpleCellCycle());
  cc->enabled_ = enabled_;
  return cc;
}

bool SimpleCellCycle::isEnabled() const {
  return enabled_;
}

void SimpleCellCycle::setEnabled(bool enabled) {
  enabled_ = enabled;
}

void SimpleCellCycle::reset() {
  cell_->divide();
}

}  // namespace cells
}  // namespace cx3d
