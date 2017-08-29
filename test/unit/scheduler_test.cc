#include "unit/scheduler_test.h"

namespace bdm {
namespace scheduler_test_internal {

TEST(SchedulerTest, NoRestoreFile) {
  ResourceManager<>::Get()->Clear();
  remove(ROOTFILE);

  // start restore validation
  TestSchedulerRestore scheduler("");
  scheduler.Simulate(100);
  EXPECT_EQ(100u, scheduler.execute_calls);
  EXPECT_EQ(0u, ResourceManager<>::Get()->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(200u, scheduler.execute_calls);
  EXPECT_EQ(0u, ResourceManager<>::Get()->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(300u, scheduler.execute_calls);
  EXPECT_EQ(0u, ResourceManager<>::Get()->Get<Cell>()->size());
}

TEST(SchedulerTest, Restore) { RunRestoreTest(); }

TEST(SchedulerTest, Backup) { RunBackupTest(); }

}  // namespace scheduler_test_internal
}  // namespace bdm
