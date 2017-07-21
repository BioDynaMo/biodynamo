#ifndef BIOLOGY_MODULE_OP_TEST_H_
#define BIOLOGY_MODULE_OP_TEST_H_

#include "biology_module_util.h"
#include "cell.h"

namespace bdm {
namespace biology_module_op_test_internal {

using std::size_t;

struct GrowthModule {
  double growth_rate_;

  GrowthModule() : growth_rate_(0.5) {}
  explicit GrowthModule(double growth_rate) : growth_rate_(growth_rate) {}

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  bool IsCopied(Event event) const { return false; }
  ClassDefNV(GrowthModule, 1);
};

typedef variant<GrowthModule> BiologyModules;

template <typename TBackend = Scalar>
using MyCell = Cell<TBackend, BiologyModules>;

}  // namespace biology_module_op_test_internal
}  // namespace bdm

#endif  // BIOLOGY_MODULE_OP_TEST_H_
