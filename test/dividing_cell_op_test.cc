#include <gtest/gtest.h>
#include "cell.h"
#include "dividing_cell_op.h"
#include "test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  using real_v = VcBackend::real_v;
  if (VcBackend::real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }
  real_v diameter;
  diameter[0] = 19;
  diameter[1] = 41;
  Cell<VcBackend> cell(diameter);
  cells->push_back(cell);

  DividingCellOp op;
  op.Compute(cells);

  auto final_diameter = (*cells)[0].GetDiameter();
  EXPECT_NEAR(19.005288996600001, final_diameter[0],
              abs_error<real_v::value_type>::value);
  EXPECT_NEAR(41, final_diameter[1], abs_error<real_v::value_type>::value);

  auto final_volume = (*cells)[0].GetVolume();
  EXPECT_NEAR(3594.3640018287319, final_volume[0],
              abs_error<real_v::value_type>::value);
  EXPECT_NEAR(36086.951213010347, final_volume[1],
              abs_error<real_v::value_type>::value);
}

TEST(DividingCellOpTest, ComputeAosoa) {
  daosoa<Cell<VcBackend>> cells;
  RunTest(&cells);
}

TEST(DividingCellOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm
