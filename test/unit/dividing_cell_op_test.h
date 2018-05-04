#ifndef UNIT_DIVIDING_CELL_OP_TEST_H_
#define UNIT_DIVIDING_CELL_OP_TEST_H_

#include <omp.h>
#include "cell.h"
#include "dividing_cell_op.h"
#include "gtest/gtest.h"
#include "unit/test_util.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

template <typename TRm = ResourceManager<>>
void RunTest() {
  Param::Reset();
  auto rm = TRm::Get();
  rm->Clear();
  auto* cells = rm->template Get<Cell>();
  // TODO(lukas) remove after https://trello.com/c/sKoOTgJM has been resolved
  omp_set_num_threads(1);  // and Rm::New<...> uses delayed push back
  cells->reserve(10);
  cells->push_back(Cell(41.0));
  cells->push_back(Cell(19.0));

  EXPECT_EQ(2u, cells->size());

  double volume_mother = (*cells)[0].GetVolume();

  DividingCellOp op;
  op(cells, 0);

  cells->Commit();

  ASSERT_EQ(3u, cells->size());
  EXPECT_NEAR(19.005288996600001, (*cells)[1].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(3594.3640018287319, (*cells)[1].GetVolume(),
              abs_error<double>::value);

  // cell got divided so it must be smaller than before
  // more detailed division test can be found in `cell_test.h`
  EXPECT_GT(41, (*cells)[0].GetDiameter());
  EXPECT_GT(41, (*cells)[2].GetDiameter());

  std::cout << (*cells)[0].GetVolume() << std::endl;
  std::cout << (*cells)[2].GetVolume() << std::endl;

  // volume of two daughter cells must be equal to volume of the mother
  EXPECT_NEAR(volume_mother, (*cells)[0].GetVolume() + (*cells)[2].GetVolume(),
              abs_error<double>::value);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

#endif  // UNIT_DIVIDING_CELL_OP_TEST_H_
