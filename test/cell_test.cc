#include "gtest/gtest.h"
#include "cell.h"

namespace bdm {

TEST(CellTest, AppendScalarCellToVectorCell) {
  Cell<ScalarBackend> cell(3.14);
  EXPECT_DOUBLE_EQ(cell.GetDiameter()[0], 3.14);
  Cell<VcBackend> cells;
  // without it SetUninitialized Append should fail, because cells is full
  cells.SetUninitialized();
  cells.Append(cell);
  EXPECT_DOUBLE_EQ(cells.GetDiameter()[0], 3.14);
}
// todo(lukas) add more tests

}  // namespace bdm
