#include "local_biology/abstract_local_biology_module.h"

namespace bdm {
namespace local_biology {

AbstractLocalBiologyModule::AbstractLocalBiologyModule() {
}

AbstractLocalBiologyModule::~AbstractLocalBiologyModule() {
}

StringBuilder& AbstractLocalBiologyModule::simStateToJson(StringBuilder& sb) const {
  sb.append("{");
//cellElement is circular reference
  return sb;
}

CellElement* AbstractLocalBiologyModule::getCellElement() const {
  return cell_element_;
}

void AbstractLocalBiologyModule::setCellElement(CellElement* cell_element) {
  cell_element_ = cell_element;
}

bool AbstractLocalBiologyModule::isCopiedWhenNeuriteBranches() const {
  return false;
}

bool AbstractLocalBiologyModule::isCopiedWhenSomaDivides() const {
  return false;
}

bool AbstractLocalBiologyModule::isCopiedWhenNeuriteElongates() const {
  return false;
}

bool AbstractLocalBiologyModule::isCopiedWhenNeuriteExtendsFromSoma() const {
  return false;
}

bool AbstractLocalBiologyModule::isDeletedAfterNeuriteHasBifurcated() const {
  return false;
}

}  // namespace local_biology
}  // namespace bdm
