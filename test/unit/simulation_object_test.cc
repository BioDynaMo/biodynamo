#include "unit/simulation_object_test.h"
#include "cell.h"
#include "unit/default_ctparam.h"

namespace bdm {
namespace simulation_object_test_internal {

TEST(SimulationObjectTest, push_backAndClear) { RunPushBackAndClearTest(); }

TEST(SimulationObjectTest, SoaGetElementIndex) {
  Rm()->Clear();
  for (uint64_t i = 0; i < 10; i++) {
    Rm()->New<Cell>(1);
  }
  Rm()->Get<Cell>()->Commit();
  EXPECT_EQ(10u, Rm()->GetNumSimObjects());
  auto cells = Rm()->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetElementIdx());
  }
}

TEST(SimulationObjectTest, Clear) {
  Rm()->Clear();
  for (uint64_t i = 0; i < 10; i++) {
    Rm()->New<Cell>(1);
  }
  auto* cells = Rm()->Get<Cell>();
  cells->Commit();

  EXPECT_EQ(10u, cells->size());

  cells->DelayedRemove(5);

  EXPECT_EQ(10u, cells->size());
  cells->clear();

  // this would segfault if `TransactionalVector::to_be_removed_` will not be
  // cleared
  cells->Commit();
}

}  // namespace simulation_object_test_internal
}  // namespace bdm
