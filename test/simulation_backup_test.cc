#include "simulation_backup.h"

#include <string>
#include "gtest/gtest.h"
#include "cell.h"
#include "io_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

class SimulationBackupTest : public ::testing::Test {};
using SimulationBackupDeathTest = SimulationBackupTest;

TEST(SimulationBackupDeathTest, GetSimulationStepsFromBackup) {
  ASSERT_DEATH({
    SimulationBackup backup("", "");
    backup.GetSimulationStepsFromBackup();
  }, ".*Requested to restore data, but no restore file given..*");
}

TEST(SimulationBackupTest, GetSimulationStepsFromBackup) {
  remove(ROOTFILE);

  IntegralTypeWrapper<size_t> wrapper(26);
  WritePersistentObject(ROOTFILE, "completed_simulation_steps", wrapper, "recreate");

  SimulationBackup backup("", ROOTFILE);
  auto iteration = backup.GetSimulationStepsFromBackup();

  EXPECT_EQ(26u, iteration);

  remove(ROOTFILE);
}

TEST(SimulationBackupDeathTest, BackupNoBackupFileSpecified) {
  ASSERT_DEATH({
    auto cells = Cell<>::NewEmptySoa();
    size_t iterations = 1;
    SimulationBackup backup("", "");
    backup.Backup(&cells, iterations);
  }, ".*Requested to backup data, but no backup file given..*");
}

TEST(SimulationBackupTest, Backup) {
  remove(ROOTFILE);

  auto cells = Cell<>::NewEmptySoa();
  cells.push_back(Cell<>());
  size_t iterations = 26;

  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(&cells, iterations);

  ASSERT_TRUE(FileExists(ROOTFILE));

  TFileRaii file(TFile::Open(ROOTFILE));

  // RuntimeVariables
  RuntimeVariables* restored_rv;
  file.Get()->GetObject(SimulationBackup::kRuntimeVariableName.c_str(), restored_rv);
  RuntimeVariables this_system;
  EXPECT_EQ(this_system, *restored_rv);

  // iterations
  IntegralTypeWrapper<size_t>* wrapper = nullptr;
  file.Get()->GetObject(SimulationBackup::kSimulationStepName.c_str(), wrapper);
  EXPECT_EQ(26u, wrapper->Get());

  // cells
  decltype(cells)* restored_cells = nullptr;
  file.Get()->GetObject(SimulationBackup::kCellName.c_str(), restored_cells);
  EXPECT_EQ(1u, restored_cells->size());
  // Writing and reading cells is tested in cell_test.h/cc

  remove(ROOTFILE);
}

TEST(SimulationBackupDeathTest, RestoreNoRestoreFileSpecified) {
  ASSERT_DEATH({
    SimulationBackup backup("", "");
    Cell<Soa>* restored_cells;
    backup.Restore(restored_cells);
  }, ".*Requested to restore data, but no restore file given..*");
}

TEST(SimulationBackupDeathTest, RestoreFileDoesNotExist) {
  ASSERT_DEATH({
    SimulationBackup backup("", "file-does-not-exist.root");
  }, ".*Given restore file does not exist.*");
}

TEST(SimulationBackupTest, BackupAndRestore) {
  remove(ROOTFILE);

  auto cells = Cell<>::NewEmptySoa();
  cells.push_back(Cell<>());
  size_t iterations = 26;

  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(&cells, iterations);

  // restore
  SimulationBackup restore("", ROOTFILE);
  //   iterations
  auto restored_iterationa = restore.GetSimulationStepsFromBackup();
  EXPECT_EQ(26u, restored_iterationa);

  //   cells
  decltype(cells)* restored_cells = nullptr;
  restore.Restore(restored_cells);
  EXPECT_EQ(1u, restored_cells->size());

  remove(ROOTFILE);
}


}  // namespace bdm
