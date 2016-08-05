#ifndef CELLS_SIMPLE_CELL_CYCLE_H_
#define CELLS_SIMPLE_CELL_CYCLE_H_

#include <memory>

#include "sim_state_serializable.h"
#include "cells/abstract_cell_module.h"

namespace bdm {
namespace cells {

class SimpleCellCycle : public AbstractCellModule {
 public:
  SimpleCellCycle();

  ~SimpleCellCycle();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /** Run the simulation*/
  void run() override;

  /** Get a copy */
  CellModule::UPtr getCopy() const override;

  bool isEnabled() const;

  void setEnabled(bool enabled);

  void reset();

 private:
  SimpleCellCycle(const SimpleCellCycle&) = delete;
  SimpleCellCycle& operator=(const SimpleCellCycle&) = delete;

  /** turned on or off */
  bool enabled_ = false;

  /* the speed of metabolic update.*/
  double dVdt_ = 150.0;

  /* the minimum size to obtain before being allowed to divide.*/
  double minimum_diameter_ = 20.0;

  ClassDefOverride(SimpleCellCycle, 1);
};

}  // namespace cells
}  // namespace bdm

#endif  // CELLS_SIMPLE_CELL_CYCLE_H_
