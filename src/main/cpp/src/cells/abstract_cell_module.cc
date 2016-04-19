#include "cells/abstract_cell_module.h"

namespace cx3d {
namespace cells {

AbstractCellModule::AbstractCellModule() {
}

AbstractCellModule::~AbstractCellModule() {
}

std::shared_ptr<Cell> AbstractCellModule::getCell() const {
  return cell_;
}

void AbstractCellModule::setCell(const std::shared_ptr<Cell>& cell) {
  cell_ = cell;
}

bool AbstractCellModule::isCopiedWhenCellDivides() const {
  return true;
}

}  // namespace cells
}  // namespace cx3d
