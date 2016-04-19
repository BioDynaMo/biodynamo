#ifndef CELLS_CELL_MODULE_H_
#define CELLS_CELL_MODULE_H_

#include <memory>

#include "sim_state_serializable.h"
#include "cells/cell.h"

namespace cx3d {
namespace cells {

class AbstractCellModule : public CellModule {
 public:
  AbstractCellModule();

  virtual ~AbstractCellModule();

  /** @return the <code>Cell</code> this module leaves in*/
  virtual std::shared_ptr<Cell> getCell() const override;

  /**@param cell the <code>Cell</code> this module lives in*/
  virtual void setCell(const std::shared_ptr<Cell>& cell) override;

  /** If returns <code>true</code>, this module is copied during cell division.*/
  virtual bool isCopiedWhenCellDivides() const override;

 protected:
  std::shared_ptr<Cell> cell_;

 private:
  AbstractCellModule(const AbstractCellModule&) = delete;
  AbstractCellModule& operator=(const AbstractCellModule&) = delete;
};

}  // namespace cells
}  // namespace cx3d

#endif  // CELLS_CELL_MODULE_H_
