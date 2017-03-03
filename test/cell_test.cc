#include "gtest/gtest.h"
#include "cell.h"

namespace bdm {

TEST(CellTest, push_backScalarCellToVectorCell) {
  Cell<ScalarBackend> cell(3.14);
  EXPECT_DOUBLE_EQ(cell.GetDiameter()[0], 3.14);
  Cell<VcBackend> cells;
  // without it SetUninitialized push_back should fail, because cells is full
  cells.SetSize(0);
  cells.push_back(cell);
  EXPECT_DOUBLE_EQ(cells.GetDiameter()[0], 3.14);
}
// todo(lukas) add more tests

}  // namespace bdm
