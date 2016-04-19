#ifndef CELLS_CELL_MODULE_H_
#define CELLS_CELL_MODULE_H_

#include <memory>

#include "sim_state_serializable.h"
#include "cells/cell.h"

namespace cx3d {
namespace cells {

//fixme change to pure virtual function after porting has been finished
/**
 * Classes implementing this interface can be added to a <code>Cell</code>, and be run.
 * They represent the biological model that CX3D is simulating.
 */
class CellModule : public SimStateSerializable {
 public:
  CellModule() {
  }

  virtual ~CellModule() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error(
        "CellModule::simStateToJson must never be called - Java must provide implementation at this point");
  }

  /** Run the simulation*/
  virtual void run() {
    throw std::logic_error("CellModule::run must never be called - Java must provide implementation at this point");
  }

  /** @return the <code>Cell</code> this module leaves in*/
  virtual std::shared_ptr<Cell> getCell() const {
    throw std::logic_error("CellModule::getCell must never be called - Java must provide implementation at this point");
  }

  /**@param cell the <code>Cell</code> this module lives in*/
  virtual void setCell(const std::shared_ptr<Cell>& cell) {
    throw std::logic_error("CellModule::setCell must never be called - Java must provide implementation at this point");
  }

  /** Get a copy */
  virtual std::shared_ptr<CellModule> getCopy() const {
    throw std::logic_error("CellModule::getCopy must never be called - Java must provide implementation at this point");
  }

  /** If returns <code>true</code>, this module is copied during cell division.*/
  virtual bool isCopiedWhenCellDivides() const {
    throw std::logic_error(
        "CellModule::isCopiedWhenCellDivides must never be called - Java must provide implementation at this point");
  }

 private:
  CellModule(const CellModule&) = delete;
  CellModule& operator=(const CellModule&) = delete;
};

}  // namespace cells
}  // namespace cx3d

#endif  // CELLS_CELL_MODULE_H_
