#ifndef CELLS_CELL_MODULE_H_
#define CELLS_CELL_MODULE_H_

#include <memory>

#include "sim_state_serializable.h"

namespace cx3d {
namespace cells {

class Cell;

/**
 * Classes implementing this interface can be added to a <code>Cell</code>, and be run.
 * They represent the biological model that CX3D is simulating.
 */
class CellModule : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<CellModule>;

  CellModule() {
  }

  virtual ~CellModule() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override = 0;

  /** Run the simulation*/
  virtual void run() = 0;

  /** @return the <code>Cell</code> this module leaves in*/
  virtual Cell* getCell() const = 0;

  /**@param cell the <code>Cell</code> this module lives in*/
  virtual void setCell(Cell* cell) = 0;

  /** Get a copy */
  virtual UPtr getCopy() const = 0;

  /** If returns <code>true</code>, this module is copied during cell division.*/
  virtual bool isCopiedWhenCellDivides() const = 0;

 private:
  CellModule(const CellModule&) = delete;
  CellModule& operator=(const CellModule&) = delete;
};

}  // namespace cells
}  // namespace cx3d

#endif  // CELLS_CELL_MODULE_H_
