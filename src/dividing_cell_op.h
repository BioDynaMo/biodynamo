#ifndef DIVIDING_CELL_OP_H_
#define DIVIDING_CELL_OP_H_

#include "backend.h"

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
      // if diameter <= 40 then changeVolume(300) else do nothing
      auto&& cell = (*cells)[i];
      if (cell.GetDiameter() <= 40) {
        cell.ChangeVolume(300);
      }
      // todo(lukas) division if diameter > 20;
    }
  }
};

}  // namespace bdm

#endif  // DIVIDING_CELL_OP_H_
