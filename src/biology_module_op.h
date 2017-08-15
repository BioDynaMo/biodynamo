#ifndef BIOLOGY_MODULE_OP_H_
#define BIOLOGY_MODULE_OP_H_

#include <cstddef>  // std::size_t
#include <cstdint>  // uint16_t

namespace bdm {

using std::size_t;

struct BiologyModuleOp {
  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      (*cells)[i].RunBiologyModules();
    }
  }
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_OP_H_
