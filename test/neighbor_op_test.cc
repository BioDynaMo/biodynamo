#include <gtest/gtest.h>
#include "cell.h"
#include "inline_vector.h"
#include "neighbor_op.h"
#include "test_util.h"

namespace bdm {

TEST(NeighborOpTest, Compute) {
  daosoa<Cell, VcBackend> cells;
  // fixme ugly
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{0, 0, 0}));
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{30, 30, 30}));
  cells.push_back(
      Cell<ScalarBackend>(std::array<ScalarBackend::real_v, 3>{60, 60, 60}));

  // execute operation
  NeighborOp op;
  op.Compute(&cells);

  // check results
  // cell 1
  auto& neighbors_1 = cells[0].GetNeighbors();
  InlineVector<int, 8> expected_1;
  expected_1.push_back(1);
  EXPECT_TRUE(expected_1 == neighbors_1[0]);
  // cell 2
  InlineVector<int, 8> expected_2;
  expected_2.push_back(0);
  expected_2.push_back(2);
  EXPECT_EQ(expected_2, neighbors_1[1]);
  // cell 3
  InlineVector<int, 8> expected_3;
  expected_3.push_back(1);
  if (VcBackend::kVecLen > 2) {
    EXPECT_EQ(expected_3, neighbors_1[2]);
  } else {
    EXPECT_EQ(expected_3, cells[1].GetNeighbors()[0]);
  }
}

}  // namespace bdm
