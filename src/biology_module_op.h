#ifndef BIOLOGY_MODULE_OP_H_
#define BIOLOGY_MODULE_OP_H_

#include <cstddef>  // std::size_t

namespace bdm {

using std::size_t;

struct BiologyModuleOp {
  template <typename TContainer>
  void operator()(TContainer* cells) const {
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      (*cells)[i].RunBiologyModules();
    }
  }
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_OP_H_
