#include "local_biology/cell_element.h"

#include <algorithm>

#include "sim_state_serialization_util.h"
#include "physics/physical_object.h"
#include "cells/cell.h"
#include "local_biology/local_biology_module.h"
#include "simulation/ecm.h"

namespace cx3d {
namespace local_biology {

std::shared_ptr<simulation::ECM> CellElement::ecm_ { nullptr };

std::size_t CellElement::id_counter_ = 0;

void CellElement::reset() {
  id_counter_ = 0;
}

void CellElement::setECM(const std::shared_ptr<cx3d::simulation::ECM>& ecm) {
  ecm_ = ecm;
}

CellElement::CellElement()
    : id_ { ++id_counter_ } {
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

bool CellElement::equalTo(const std::shared_ptr<CellElement>& other) const {
  return this == other.get();
}

void CellElement::addLocalBiologyModule(const std::shared_ptr<LocalBiologyModule>& m) {
  local_biology_modules_.push_back(m);
  m->setCellElement(shared_from_this());  // set the callback
}

void CellElement::removeLocalBiologyModule(const std::shared_ptr<LocalBiologyModule> m) {
  auto it = std::find(local_biology_modules_.begin(), local_biology_modules_.end(), m);
  local_biology_modules_.erase(it);
}

void CellElement::cleanAllLocalBiologyModules() {
  local_biology_modules_.clear();
}

std::list<std::shared_ptr<LocalBiologyModule>> CellElement::getLocalBiologyModulesList() {
  //fixme return vector once porting is finished
  std::list<std::shared_ptr<LocalBiologyModule>> list;
  for (auto module : local_biology_modules_) {
    list.push_back(module);
  }
  return list;
}

void CellElement::setLocalBiologyModulesList(const std::list<std::shared_ptr<LocalBiologyModule>>& modules) {
  //fixme use vector once porting is finished
  local_biology_modules_.clear();
  for (auto module : modules) {
    local_biology_modules_.push_back(module);
  }
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
