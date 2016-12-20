#include <gtest/gtest.h>
#include "cell.h"
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
  bdm::array<int, 8> expected;
  expected[0] = 1;
  expected.SetSize(1);
  EXPECT_EQ(expected, neighbors_1[0]);
  // cell 2
  expected[0] = 0;
  expected[1] = 2;
  expected.SetSize(2);
  EXPECT_EQ(expected, neighbors_1[1]);
  // cell 3
  expected[0] = 1;
  expected.SetSize(1);
  if (VcBackend::kVecLen > 2) {
    EXPECT_EQ(expected, neighbors_1[2]);
  } else {
    EXPECT_EQ(expected, cells[1].GetNeighbors()[0]);
  }
}

}  // namespace bdm