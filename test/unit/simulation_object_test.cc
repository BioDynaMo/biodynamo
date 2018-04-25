#include "unit/simulation_object_test.h"
#include "cell.h"
#include "unit/default_ctparam.h"

namespace bdm {
namespace simulation_object_test_internal {

TEST(SimulationObjectTest, push_backAndClear) { RunPushBackAndClearTest(); }

TEST(SimulationObjectTest, SoaGetElementIndex) {
  Rm()->Clear();
  for (uint64_t i = 0; i < 10; i++) {
    Rm()->template New<Cell>(1);
  }
  EXPECT_EQ(10u, Rm()->GetNumSimObjects());
  auto cells = Rm()->template Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetElementIdx());
  }
}

}  // namespace simulation_object_test_internal
}  // namespace bdm
