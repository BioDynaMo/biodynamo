#ifndef DIVIDING_CELL_OP_H_
#define DIVIDING_CELL_OP_H_

#include "backend.h"
#include "make_thread_safe.h"

namespace bdm {

class DividingCellOp {
 public:
  DividingCellOp() {}
  ~DividingCellOp() {}
  DividingCellOp(const DividingCellOp&) = delete;
  DividingCellOp& operator=(const DividingCellOp&) = delete;

  template <typename TContainer>
  void Compute(TContainer* cells) const {
#pragma omp parallel
    {
      auto thread_safe_cells = make_thread_safe(cells);
      const size_t n_vectors = thread_safe_cells->Vectors();
#pragma omp for
      for (size_t i = 0; i < n_vectors; i++) {
        // if diameter <= 20 then changeVolume(300) else do nothing
        auto ifresult = (*thread_safe_cells)[i].GetDiameter() <= 40;
        VcVectorBackend::real_v dv(300);
        dv.setZeroInverted(ifresult);
        (*thread_safe_cells)[i].ChangeVolume(dv);
        // todo(lukas) division if diameter > 20;
      }
    }
  }
};

}  // namespace bdm

#endif  // DIVIDING_CELL_OP_H_
