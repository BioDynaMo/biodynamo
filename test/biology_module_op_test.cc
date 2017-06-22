#include "biology_module_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "test_util.h"
#include "transactional_vector.h"

namespace bdm {
namespace biology_module_op_test_internal {

using std::size_t;

struct GrowthModule {
  double growth_rate_;

  explicit GrowthModule(double growth_rate) : growth_rate_(growth_rate) {}

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  bool IsCopied(Event event) const { return false; }
};

}  // namespace biology_module_op_test_internal

BDM_DEFINE_BIOLOGY_MODULES(biology_module_op_test_internal::GrowthModule);

namespace biology_module_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  Cell cell_1(12);
  cell_1.AddBiologyModule(GrowthModule(2));

  Cell cell_2(34);
  cell_2.AddBiologyModule(GrowthModule(3));

  cells->push_back(cell_1);
  cells->push_back(cell_2);

  BiologyModuleOp op;
  op.Compute(cells);

  EXPECT_EQ(2u, cells->size());
  EXPECT_NEAR(14, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(37, (*cells)[1].GetDiameter(), abs_error<double>::value);
}

TEST(BiologyModuleOpTest, ComputeAos) {
  TransactionalVector<Cell> cells;
  RunTest(&cells);
}

TEST(BiologyModuleOpTest, ComputeSoa) {
  auto cells = Cell::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace biology_module_op_test_internal
}  // namespace bdm
