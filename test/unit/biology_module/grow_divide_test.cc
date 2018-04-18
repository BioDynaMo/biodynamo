#include "biology_module/grow_divide.h"
#include "gtest/gtest.h"
#include "resource_manager.h"
#include "unit/test_util.h"

namespace bdm {

TEST(GrowDivideTest, Grow) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  Param::Reset();

  Cell cell;
  cell.SetDiameter(40);

  GrowDivide gd(40, 300, {gAllBmEvents});
  gd.Run(&cell);

  rm->Get<Cell>()->Commit();

  EXPECT_NEAR(33513.321638291127, cell.GetVolume(), abs_error<double>::value);
  EXPECT_EQ(0u, rm->Get<Cell>()->size());
}

TEST(GrowDivideTest, Divide) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  Param::Reset();

  Cell cell;
  cell.SetDiameter(41);

  GrowDivide gd(40, 300, {gAllBmEvents});
  gd.Run(&cell);

  rm->Get<Cell>()->Commit();

  EXPECT_GT(41, cell.GetDiameter());
  EXPECT_EQ(1u, rm->Get<Cell>()->size());
}

}  // namespace bdm
