#include "local_biology/cell_element.h"

#include <algorithm>

#include "sim_state_serialization_util.h"
#include "physics/physical_object.h"
#include "cells/cell.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"

namespace cx3d {
namespace local_biology {

ECM* CellElement::ecm_ = ECM::getInstance();

std::size_t CellElement::id_counter_ = 0;

void CellElement::reset() {
  id_counter_ = 0;
}

CellElement::CellElement()
    : id_ { ++id_counter_ } {
}

CellElement::~CellElement() {
  for (auto module : local_biology_modules_) {
    delete module;
  }
}

StringBuilder& CellElement::simStateToJson(StringBuilder& sb) const {
  sb.append("{");
  SimStateSerializationUtil::keyValue(sb, "ID", id_);
  SimStateSerializationUtil::keyValue(sb, "idCounter", id_counter_);
  SimStateSerializationUtil::unorderedCollection(sb, "localBiologyModules", local_biology_modules_);
  return sb;
}

std::string CellElement::toString() const {
  return "CE";
}

void CellElement::addLocalBiologyModule(LocalBiologyModule::UPtr m) {
  local_biology_modules_.push_back(m.get());
  m->setCellElement(this);  // set the callback
  m.release();
}

void CellElement::removeLocalBiologyModule(LocalBiologyModule* m) {
  auto it = std::find(local_biology_modules_.begin(), local_biology_modules_.end(), m);
  local_biology_modules_.erase(it);
  delete m;
}

void CellElement::cleanAllLocalBiologyModules() {
  for (auto module : local_biology_modules_) {
    delete module;
  }
  local_biology_modules_.clear();
}

std::vector<LocalBiologyModule*> CellElement::getLocalBiologyModulesList() {
  return local_biology_modules_;
}

void CellElement::setCell(Cell* c) {
  cell_ = c;
}

Cell* CellElement::getCell() const {
  return cell_;
}

std::array<double, 3> CellElement::getLocation() {
  return getPhysical()->getMassLocation();
}

void CellElement::move(double speed, std::array<double, 3>& direction) {
  getPhysical()->movePointMass(speed, direction);
}

void CellElement::runLocalBiologyModules() {
  for (auto i = 0; i < local_biology_modules_.size(); i++) {
    local_biology_modules_[i]->run();
  }
}

}  // namespace local_biology
}  // namespace cx3d
