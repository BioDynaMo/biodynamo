#ifndef UNIT_DIVIDING_CELL_OP_TEST_H_
#define UNIT_DIVIDING_CELL_OP_TEST_H_

#include "dividing_cell_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "unit/test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

template <typename T, typename TRm = ResourceManager<>>
void RunTest(T* cells) {
  TRm::Get()->Clear();
  cells->push_back(Cell(41.0));
  cells->push_back(Cell(19.0));

  double volume_mother = (*cells)[0].GetVolume();

  DividingCellOp op;
  op(cells, 0);

  EXPECT_EQ(3u, cells->size());
  EXPECT_NEAR(19.005288996600001, (*cells)[1].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(3594.3640018287319, (*cells)[1].GetVolume(),
              abs_error<double>::value);

  // cell got divided so it must be smaller than before
  // more detailed division test can be found in `cell_test.h`
  EXPECT_GT(41, (*cells)[0].GetDiameter());
  EXPECT_GT(41, (*cells)[2].GetDiameter());
  // volume of two daughter cells must be equal to volume of the mother
  EXPECT_NEAR(volume_mother, (*cells)[0].GetVolume() + (*cells)[2].GetVolume(),
              abs_error<double>::value);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

#endif  // UNIT_DIVIDING_CELL_OP_TEST_H_
