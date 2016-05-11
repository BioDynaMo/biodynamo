#ifndef CELLS_SIMPLE_CELL_CYCLE_H_
#define CELLS_SIMPLE_CELL_CYCLE_H_

#include <memory>

#include "sim_state_serializable.h"
#include "cells/abstract_cell_module.h"

namespace cx3d {
namespace cells {

class SimpleCellCycle : public AbstractCellModule {
 public:
  SimpleCellCycle();

  virtual ~SimpleCellCycle();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /** Run the simulation*/
  virtual void run() override;

  /** Get a copy */
  virtual CellModule::UPtr getCopy() const override;

  virtual bool isEnabled() const;

  virtual void setEnabled(bool enabled);

  virtual void reset();

 private:
  SimpleCellCycle(const SimpleCellCycle&) = delete;
  SimpleCellCycle& operator=(const SimpleCellCycle&) = delete;

  /** turned on or off */
  bool enabled_ = false;

  /* the speed of metabolic update.*/
  double dVdt_ = 150.0;

  /* the minimum size to obtain before being allowed to divide.*/
  double minimum_diameter_ = 20.0;
};

}  // namespace cells
}  // namespace cx3d

#endif  // CELLS_SIMPLE_CELL_CYCLE_H_
