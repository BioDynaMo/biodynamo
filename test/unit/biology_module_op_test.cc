#include "biology_module_op.h"
#include "gtest/gtest.h"
#include "transactional_vector.h"
#include "unit/biology_module_op_test.h"

namespace bdm {
namespace biology_module_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  TestCell<CompileTimeParam> cell_1(12);
  cell_1.AddBiologyModule(GrowthModule(2));

  TestCell<CompileTimeParam> cell_2(34);
  cell_2.AddBiologyModule(GrowthModule(3));

  cells->push_back(cell_1);
  cells->push_back(cell_2);

  BiologyModuleOp op;
  op(cells, 0);

  EXPECT_EQ(2u, cells->size());
  EXPECT_NEAR(14, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(37, (*cells)[1].GetDiameter(), abs_error<double>::value);
}

TEST(BiologyModuleOpTest, ComputeAos) {
  TransactionalVector<TestCell<CompileTimeParam>> cells;
  RunTest(&cells);
}

TEST(BiologyModuleOpTest, ComputeSoa) {
  auto cells = TestCell<CompileTimeParam>::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace biology_module_op_test_internal
}  // namespace bdm
