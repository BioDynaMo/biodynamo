// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "simulation_backup.h"

#include <string>
#include "cell.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"
#include "bdm_imp.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

class SimulationBackupTest : public ::testing::Test {};
using SimulationBackupDeathTest = SimulationBackupTest;

TEST(SimulationBackupDeathTest, GetSimulationStepsFromBackup) {
  ASSERT_DEATH(
      {
        SimulationBackup backup("", "");
        backup.GetSimulationStepsFromBackup();
      },
      ".*Requested to restore data, but no restore file given..*");
}

TEST(SimulationBackupTest, GetSimulationStepsFromBackup) {
  remove(ROOTFILE);

  IntegralTypeWrapper<size_t> wrapper(26);
  WritePersistentObject(ROOTFILE, "completed_simulation_steps", wrapper,
                        "recreate");

  SimulationBackup backup("", ROOTFILE);
  auto iteration = backup.GetSimulationStepsFromBackup();

  EXPECT_EQ(26u, iteration);

  remove(ROOTFILE);
}

TEST(SimulationBackupDeathTest, BackupNoBackupFileSpecified) {
  ASSERT_DEATH(
      {
        auto cells = Cell::NewEmptySoa();
        size_t iterations = 1;
        SimulationBackup backup("", "");
        backup.Backup(iterations);
      },
      ".*Requested to backup data, but no backup file given..*");
}

TEST(SimulationBackupTest, Backup) {
  remove(ROOTFILE);
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  auto cells = rm->template Get<Cell>();
  cells->push_back(Cell());
  size_t iterations = 26;

  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(iterations);

  ASSERT_TRUE(FileExists(ROOTFILE));

  TFileRaii file(TFile::Open(ROOTFILE));

  // RuntimeVariables
  RuntimeVariables* restored_rv;
  file.Get()->GetObject(SimulationBackup::kRuntimeVariableName.c_str(),
                        restored_rv);
  RuntimeVariables this_system;
  EXPECT_EQ(this_system, *restored_rv);

  // iterations
  IntegralTypeWrapper<size_t>* wrapper = nullptr;
  file.Get()->GetObject(SimulationBackup::kSimulationStepName.c_str(), wrapper);
  EXPECT_EQ(26u, wrapper->Get());

  // ResourceManager
  ResourceManager<>* restored_rm = nullptr;
  file.Get()->GetObject(SimulationBackup::kResouceManagerName.c_str(),
                        restored_rm);
  EXPECT_EQ(1u, restored_rm->Get<Cell>()->size());
  // Writing and reading ResourceManager is tested in resource_manager_test.cc

  delete restored_rm;

  remove(ROOTFILE);
}

TEST(SimulationBackupDeathTest, RestoreNoRestoreFileSpecified) {
  ASSERT_DEATH(
      {
        SimulationBackup backup("", "");
        backup.Restore();
      },
      ".*Requested to restore data, but no restore file given..*");
}

TEST(SimulationBackupDeathTest, RestoreFileDoesNotExist) {
  ASSERT_DEATH({ SimulationBackup backup("", "file-does-not-exist.root"); },
               ".*Given restore file does not exist.*");
}

TEST(SimulationBackupTest, BackupAndRestore) {
  remove(ROOTFILE);
  BdmSim<> simulation(typeid(*this).name());
  auto* rm = simulation.GetRm();

  auto cells = rm->Get<Cell>();
  cells->push_back(Cell());
  size_t iterations = 26;

  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(iterations);

  rm->Clear();

  // restore
  SimulationBackup restore("", ROOTFILE);
  //   iterations
  auto restored_iterationa = restore.GetSimulationStepsFromBackup();
  EXPECT_EQ(26u, restored_iterationa);

  //   ResourceManager
  restore.Restore();
  EXPECT_EQ(1u, rm->Get<Cell>()->size());

  remove(ROOTFILE);
}

}  // namespace bdm
