#include "dividing_cell_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  cells->push_back(Cell<>(19.0));
  cells->push_back(Cell<>(41.0));

  DividingCellOp op;
  op.Compute(cells);

  EXPECT_NEAR(19.005288996600001, (*cells)[0].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(41, (*cells)[1].GetDiameter(), abs_error<double>::value);

  EXPECT_NEAR(3594.3640018287319, (*cells)[0].GetVolume(),
              abs_error<double>::value);
  EXPECT_NEAR(36086.951213010347, (*cells)[1].GetVolume(),
              abs_error<double>::value);
}

TEST(DividingCellOpTest, ComputeAos) {
  std::vector<Cell<Scalar>> cells;
  RunTest(&cells);
}

TEST(DividingCellOpTest, ComputeSoa) {
  Cell<Soa> cells;
  RunTest(&cells);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm
