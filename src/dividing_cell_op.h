#ifndef DIVIDING_CELL_OP_H_
#define DIVIDING_CELL_OP_H_

#include "backend.h"
#include "simulation_object_util.h"

namespace bdm {

class DividingCellOp {
 public:
  DividingCellOp() {}
  ~DividingCellOp() {}
  DividingCellOp(const DividingCellOp&) = delete;
  DividingCellOp& operator=(const DividingCellOp&) = delete;

  template <typename TContainer>
  void Compute(TContainer* cells) const {
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      if (cell.GetDiameter() <= 40) {
        cell.ChangeVolume(300);
      } else {
        Divide(cell, cells);
      }
    }
    cells->Commit();
  }
};

}  // namespace bdm

#endif  // DIVIDING_CELL_OP_H_
