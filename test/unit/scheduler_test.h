#ifndef UNIT_SCHEDULER_TEST_H_
#define UNIT_SCHEDULER_TEST_H_

#include <string>
#include "cell.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "scheduler.h"
#include "unistd.h"
#include "unit/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace scheduler_test_internal {

class TestSchedulerRestore : public Scheduler<> {
 public:
  static TestSchedulerRestore Create(const std::string& restore) {
    Param::backup_file_ = "";
    Param::restore_file_ = restore;
    return TestSchedulerRestore();
  }

  void Execute() override { execute_calls++; }

  unsigned execute_calls = 0;

 private:
  TestSchedulerRestore() : Scheduler<>() {}
};

class TestSchedulerBackup : public Scheduler<> {
 public:
  static TestSchedulerBackup Create(const std::string& backup) {
    Param::backup_file_ = backup;
    Param::restore_file_ = "";
    return TestSchedulerBackup();
  }

  void Execute() override {
    // sleep
    usleep(350000);
    // backup should be created every second -> every three iterations
    if (execute_calls_ % 3 != 0 || execute_calls_ == 0) {
      EXPECT_FALSE(FileExists(ROOTFILE));
    } else {
      EXPECT_TRUE(FileExists(ROOTFILE));
      remove(ROOTFILE);
    }
    execute_calls_++;
  }
  unsigned execute_calls_ = 0;

 private:
  TestSchedulerBackup() : Scheduler<>() {}
};

inline void RunRestoreTest() {
  ResourceManager<>::Get()->Clear();
  remove(ROOTFILE);

  // create backup that will be restored later on
  Cell cell;
  cell.SetDiameter(10);  // important for grid to determine box size
  ResourceManager<>::Get()->Get<Cell>()->push_back(cell);
  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(149);
  ResourceManager<>::Get()->Clear();
  EXPECT_EQ(0u, ResourceManager<>::Get()->Get<Cell>()->size());

  // start restore validation
  auto scheduler = TestSchedulerRestore::Create(ROOTFILE);
  // 149 simulation steps have already been calculated. Therefore, this call
  // should be ignored
  scheduler.Simulate(100);
  EXPECT_EQ(0u, scheduler.execute_calls);
  EXPECT_EQ(0u, ResourceManager<>::Get()->Get<Cell>()->size());

  // Restore should happen within this call
  scheduler.Simulate(100);
  //   only 51 steps should be simulated
  EXPECT_EQ(51u, scheduler.execute_calls);
  EXPECT_EQ(1u, ResourceManager<>::Get()->Get<Cell>()->size());

  // add element to see if if restore happens again
  ResourceManager<>::Get()->Get<Cell>()->push_back(Cell());

  // normal simulation - no restore
  scheduler.Simulate(100);
  EXPECT_EQ(151u, scheduler.execute_calls);
  EXPECT_EQ(2u, ResourceManager<>::Get()->Get<Cell>()->size());

  remove(ROOTFILE);
}

inline void RunBackupTest() {
  ResourceManager<>::Get()->Clear();
  remove(ROOTFILE);

  Cell cell;
  cell.SetDiameter(10);  // important for grid to determine box size
  ResourceManager<>::Get()->Get<Cell>()->push_back(cell);

  auto scheduler = TestSchedulerBackup::Create(ROOTFILE);
  Param::backup_interval_ = 1;
  // one simulation step takes 350 ms -> backup should be created every three
  // steps
  scheduler.Simulate(7);
  remove(ROOTFILE);
}

}  // namespace scheduler_test_internal
}  // namespace bdm

#endif  // UNIT_SCHEDULER_TEST_H_
