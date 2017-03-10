#ifndef DIVIDING_CELL_OP_H_
#define DIVIDING_CELL_OP_H_

#include "backend_old.h"
#include "cell.h"
#include "daosoa.h"

namespace bdm {

class DividingCellOp {
 public:
  DividingCellOp() {}
  ~DividingCellOp() {}
  DividingCellOp(const DividingCellOp&) = delete;
  DividingCellOp& operator=(const DividingCellOp&) = delete;

  template <typename daosoa>
  Vc_ALWAYS_INLINE void Compute(daosoa* cells) const {
    const size_t n_vectors = cells->vectors();
#pragma omp parallel for
    for (size_t i = 0; i < n_vectors; i++) {
      // if diameter <= 20 then changeVolume(300) else do nothing
      auto ifresult = (*cells)[i].GetDiameter() <= 40;
      VcBackend::real_v dv(300);
      dv.setZeroInverted(ifresult);
      (*cells)[i].ChangeVolume(dv);
      // todo(lukas) division if diameter > 20;
    }
  }
};

}  // namespace bdm

#endif  // DIVIDING_CELL_OP_H_
