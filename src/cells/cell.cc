#include "cells/cell.h"

#include "matrix.h"
#include "sim_state_serialization_util.h"
#include "physics/physical_sphere.h"
#include "simulation/ecm.h"

namespace cx3d {
namespace cells {

int Cell::id_counter_ = 0;
std::shared_ptr<simulation::ECM> Cell::ecm_ { nullptr };

void Cell::reset() {
  id_counter_ = 0;
}

Cell::Cell()
    : id_ { ++id_counter_ } {
}

Cell::~Cell() {
}

StringBuilder& Cell::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::keyValue(sb, "id", id_);
  SimStateSerializationUtil::keyValue(sb, "idCounter", id_counter_);
  SimStateSerializationUtil::unorderedCollection(sb, "cellModules", cell_modules_);
  SimStateSerializationUtil::keyValue(sb, "somaElement", soma_);
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

bool Cell::equalTo(const std::shared_ptr<Cell>& other) const {
  return this == other.get();
}

void Cell::run() {
  // Run all the CellModules
  // Important : the vector might be modified during the loop (for instance if a module deletes itself)
  for (auto i = 0; i < cell_modules_.size(); i++) {
    auto module = cell_modules_[i];
    module->run();
  }
}

std::shared_ptr<Cell> Cell::divide() {
  // find a volume ration close to 1;
  return divide(0.9 + 0.2 * ecm_->getRandomDouble1());
}

std::shared_ptr<Cell> Cell::divide(double volume_ratio) {
  // find random point on sphere (based on : http://mathworld.wolfram.com/SpherePointPicking.html)
  double theta = 6.28318531 * ecm_->getRandomDouble1();
  double phi = ecm_->acos(2 * ecm_->getRandomDouble1() - 1);
  return divide(volume_ratio, phi, theta);
}

std::shared_ptr<Cell> Cell::divide(const std::array<double, 3>& axis) {
  auto sphere = soma_->getPhysicalSphere();
  auto polarcoord = sphere->transformCoordinatesGlobalToPolar(Matrix::add(axis, sphere->getMassLocation()));
  return divide(0.9 + 0.2 * ecm_->getRandomDouble1(), polarcoord[1], polarcoord[2]);
}

std::shared_ptr<Cell> Cell::divide(double volume_ratio, const std::array<double, 3>& axis) {
  auto sphere = soma_->getPhysicalSphere();
  auto polarcoord = sphere->transformCoordinatesGlobalToPolar(Matrix::add(axis, sphere->getMassLocation()));
  return divide(volume_ratio, polarcoord[1], polarcoord[2]);
}

std::shared_ptr<Cell> Cell::divide(double volume_ratio, double phi, double theta) {
  // 1) Create a new daughter cell. The mother cell and the 1st daughter cell are the same java object instance!
  auto newCell = create();
  id_ = ++id_counter_;  // todo why is a new id assigned here?

  // 2) Copy the CellModules that have to be copied
  for (auto module : cell_modules_) {
    if (module->isCopiedWhenCellDivides()) {
      newCell->addCellModule(module->getCopy());
    }
  }

  // 3) Also divide the LocalBiologyLayer
  newCell->setSomaElement(soma_->divide(volume_ratio, phi, theta));
  return newCell;
}

void Cell::addCellModule(const std::shared_ptr<CellModule>& m) {
  cell_modules_.push_back(m);
  m->setCell(shared_from_this());
}

void Cell::removeCellModule(const std::shared_ptr<CellModule>& m) {
  auto it = std::find(cell_modules_.begin(), cell_modules_.end(), m);
  cell_modules_.erase(it);
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

std::shared_ptr<local_biology::SomaElement> Cell::getSomaElement() const {
  return soma_;
}

void Cell::setSomaElement(const std::shared_ptr<local_biology::SomaElement>& soma) {
  soma_ = soma;
  soma_->setCell(shared_from_this());
}

int Cell::getID() const {
  return id_;
}

std::vector<std::shared_ptr<CellModule> > Cell::getCellModules() const {
  return cell_modules_;
}

std::list<std::shared_ptr<local_biology::NeuriteElement>> Cell::getNeuriteElements() const {
  std::list<std::shared_ptr<local_biology::NeuriteElement>> neurite_elements;
  for (auto ne : soma_->getNeuriteList()) {
    ne->addYourselfAndDistalNeuriteElements(neurite_elements);
  }
  return neurite_elements;
}

void Cell::init() {
  ecm_->addCell(shared_from_this());
}

}  // namespace cells
}  // namespace cx3d

