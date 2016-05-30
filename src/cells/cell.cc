#include "cells/cell.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "physics/physical_sphere.h"
#include "simulation/ecm.h"

namespace cx3d {
namespace cells {

using simulation::ECM;

int Cell::id_counter_ = 0;
ECM* Cell::ecm_ = ECM::getInstance();

void Cell::reset() {
  id_counter_ = 0;
}

Cell::Cell()
    : id_ { ++id_counter_ } {
  ecm_->addCell(std::unique_ptr < Cell > (this));
}

Cell::~Cell() {
}

StringBuilder& Cell::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::keyValue(sb, "id", id_);
  SimStateSerializationUtil::keyValue(sb, "idCounter", id_counter_);
  SimStateSerializationUtil::unorderedCollection(sb, "cellModules", cell_modules_);
  SimStateSerializationUtil::keyValue(sb, "somaElement", soma_.get());
  SimStateSerializationUtil::unorderedCollection(sb, "neuriteRootList", neurite_root_list_);
  std::string neuro_type = "";
  switch (neuro_ml_type_) {
    case kExcitatatory:
      neuro_type = "Excitatory_cells";
      break;
    case kInhibitory:
      neuro_type = "Inhibitory_cells";
      break;
  }
  SimStateSerializationUtil::keyValue(sb, "neuroMlType", neuro_type, true);
  SimStateSerializationUtil::keyValue(sb, "type", type_, true);

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

std::string Cell::toString() const {
  return "Cell";
}

void Cell::run() {
  // Run all the CellModules
  // Important : the vector might be modified during the loop (for instance if a module deletes itself)
  for (auto i = 0; i < cell_modules_.size(); i++) {
    cell_modules_[i]->run();
  }
}

Cell* Cell::divide() {
  // find a volume ration close to 1;
  return divide(0.9 + 0.2 * Random::nextDouble());
}

Cell* Cell::divide(double volume_ratio) {
  // find random point on sphere (based on : http://mathworld.wolfram.com/SpherePointPicking.html)
  double theta = 6.28318531 * Random::nextDouble();
  double phi = MathUtil::acos(2 * Random::nextDouble() - 1);
  return divide(volume_ratio, phi, theta);
}

Cell* Cell::divide(const std::array<double, 3>& axis) {
  auto sphere = soma_->getPhysicalSphere();
  auto polarcoord = sphere->transformCoordinatesGlobalToPolar(Matrix::add(axis, sphere->getMassLocation()));
  return divide(0.9 + 0.2 * Random::nextDouble(), polarcoord[1], polarcoord[2]);
}

Cell* Cell::divide(double volume_ratio, const std::array<double, 3>& axis) {
  auto sphere = soma_->getPhysicalSphere();
  auto polarcoord = sphere->transformCoordinatesGlobalToPolar(Matrix::add(axis, sphere->getMassLocation()));
  return divide(volume_ratio, polarcoord[1], polarcoord[2]);
}

Cell* Cell::divide(double volume_ratio, double phi, double theta) {
  // 1) Create a new daughter cell. The mother cell and the 1st daughter cell are the same java object instance!
  auto new_cell = new Cell();
  id_ = ++id_counter_;  // todo why is a new id assigned here?

  // 2) Copy the CellModules that have to be copied
  for (auto& module : cell_modules_) {
    if (module->isCopiedWhenCellDivides()) {
      new_cell->addCellModule(module->getCopy());
    }
  }

  // 3) Also divide the LocalBiologyLayer
  new_cell->setSomaElement(soma_->divide(volume_ratio, phi, theta));
  return new_cell;
}

void Cell::addCellModule(CellModule::UPtr m) {
  m->setCell(this);
  cell_modules_.push_back(move(m));
}

void Cell::removeCellModule(CellModule* m) {
  STLUtil::vectorRemove(cell_modules_, m);
}

void Cell::cleanAllCellModules() {
  cell_modules_.clear();
}

void Cell::setColorForAllPhysicalObjects(Color color) {
  soma_->getPhysical()->setColor(color);
  for (auto ne : getNeuriteElements()) {
    ne->getPhysical()->setColor(color);
  }
}

void Cell::setNeuroMLType(Cell::NeuroMLType neuro_ml_type) {
  neuro_ml_type_ = neuro_ml_type;
}

Cell::NeuroMLType Cell::getNeuroMLType() const {
  return neuro_ml_type_;
}

std::string Cell::getType() const {
  return type_;
}

void Cell::setType(const std::string& type) {
  type_ = type;
}

SomaElement* Cell::getSomaElement() const {
  return soma_.get();
}

void Cell::setSomaElement(SomaElement::UPtr soma) {
  soma_ = std::move(soma);
  soma_->setCell(this);
}

int Cell::getID() const {
  return id_;
}

std::list<NeuriteElement*> Cell::getNeuriteElements() const {
  std::list<NeuriteElement*> neurite_elements;
  for (auto ne : soma_->getNeuriteList()) {
    ne->addYourselfAndDistalNeuriteElements(neurite_elements);
  }
  return neurite_elements;
}

void Cell::addNeuriteElement(NeuriteElement::UPtr neurite) {
  neurites_.push_back(std::move(neurite));
}

}  // namespace cells
}  // namespace cx3d

