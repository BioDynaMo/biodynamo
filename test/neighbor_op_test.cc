#include "neighbor_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "inline_vector.h"
#include "neighbor_nanoflann_op.h"
#include "test_util.h"

namespace bdm {

BDM_DEFAULT_BIOLOGY_MODULES();

namespace neighbor_op_test_internal {

template <typename T, typename Op>
void RunTest(T* cells, const Op& op) {
  cells->push_back(Cell({0, 0, 0}));
  cells->push_back(Cell({30, 30, 30}));
  cells->push_back(Cell({60, 60, 60}));

  // execute operation
  op.Compute(cells);

  // check results
  // cell 1
  InlineVector<int, 8> expected_1;
  expected_1.push_back(1);
  EXPECT_TRUE(expected_1 == (*cells)[0].GetNeighbors());
  // cell 2
  InlineVector<int, 8> expected_2;
  expected_2.push_back(0);
  expected_2.push_back(2);
  EXPECT_EQ(expected_2, (*cells)[1].GetNeighbors());
  // cell 3
  InlineVector<int, 8> expected_3;
  expected_3.push_back(1);
  EXPECT_EQ(expected_3, (*cells)[2].GetNeighbors());
}

TEST(NeighborOpTest, ComputeAosoa) {
  std::vector<Cell> cells;
  RunTest(&cells, NeighborOp());
}

TEST(NeighborOpTest, ComputeSoa) {
  auto cells = Cell::NewEmptySoa();
  RunTest(&cells, NeighborOp());
}

TEST(NeighborNanoflannOpTest, ComputeAosoa) {
  std::vector<Cell> cells;
  RunTest(&cells, NeighborNanoflannOp());
}

TEST(NeighborNanoflannOpTest, ComputeSoa) {
  auto cells = Cell::NewEmptySoa();
  RunTest(&cells, NeighborNanoflannOp());
}

}  // namespace neighbor_op_test_internal
}  // namespace bdm
