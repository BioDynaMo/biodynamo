#include "unit/scheduler_test.h"

namespace bdm {
namespace scheduler_test_internal {

TEST(SchedulerTest, NoRestoreFile) {
  ResourceManager<>::Get()->Clear();
  remove(ROOTFILE);

  Cell cell;
  cell.SetDiameter(10);  // important for grid to determine box size
  ResourceManager<>::Get()->Get<Cell>()->push_back(cell);

  // start restore validation
  auto scheduler = TestSchedulerRestore::Create("");
  scheduler.Simulate(100);
  EXPECT_EQ(100u, scheduler.execute_calls);
  EXPECT_EQ(1u, ResourceManager<>::Get()->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(200u, scheduler.execute_calls);
  EXPECT_EQ(1u, ResourceManager<>::Get()->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(300u, scheduler.execute_calls);
  EXPECT_EQ(1u, ResourceManager<>::Get()->Get<Cell>()->size());
}

TEST(SchedulerTest, Restore) { RunRestoreTest(); }

TEST(SchedulerTest, Backup) { RunBackupTest(); }

}  // namespace scheduler_test_internal
}  // namespace bdm
