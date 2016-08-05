#ifndef CELLS_ABSTRACT_CELL_MODULE_H_
#define CELLS_ABSTRACT_CELL_MODULE_H_

#include <memory>

#include "sim_state_serializable.h"
#include "cells/cell_module.h"

namespace bdm {
namespace cells {

class AbstractCellModule : public CellModule {
 public:
  AbstractCellModule();

  virtual ~AbstractCellModule();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /** @return the <code>Cell</code> this module leaves in*/
  Cell* getCell() const override;

  /**@param cell the <code>Cell</code> this module lives in*/
  void setCell(Cell* cell) override;

  /** If returns <code>true</code>, this module is copied during cell division.*/
  bool isCopiedWhenCellDivides() const override;

 protected:
  Cell* cell_ = nullptr;

 private:
  AbstractCellModule(const AbstractCellModule&) = delete;
  AbstractCellModule& operator=(const AbstractCellModule&) = delete;

  ClassDefOverride(AbstractCellModule, 1);
};

}  // namespace cells
}  // namespace bdm

#endif  // CELLS_ABSTRACT_CELL_MODULE_H_
