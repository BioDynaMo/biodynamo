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

#include "core/simulation_backup.h"

#include <string>
#include "core/resource_manager.h"
#include "core/agent/cell.h"
#include "core/util/io.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#define ROOTFILE "bdmFile.root"

#ifdef USE_DICT

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
        size_t iterations = 1;
        SimulationBackup backup("", "");
        backup.Backup(iterations);
      },
      ".*Requested to backup data, but no backup file given..*");
}

TEST(SimulationBackupTest, Backup) {
  remove(ROOTFILE);
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new Cell());
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

  // Simulation
  Simulation* restored_simulation = nullptr;
  file.Get()->GetObject(SimulationBackup::kSimulationName.c_str(),
                        restored_simulation);
  EXPECT_EQ(1u, restored_simulation->GetResourceManager()->GetNumAgents());
  // Writing and reading Simulation is tested in simulation_test.cc

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
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new Cell());
  size_t iterations = 26;

  SimulationBackup backup(ROOTFILE, "");
  backup.Backup(iterations);

  // restore
  SimulationBackup restore("", ROOTFILE);
  //   iterations
  auto restored_iterations = restore.GetSimulationStepsFromBackup();
  EXPECT_EQ(26u, restored_iterations);
  restore.Restore();

  //   ResourceManager should not have changed
  EXPECT_EQ(rm, simulation.GetResourceManager());

  //   get new ResourceManager
  rm = simulation.GetResourceManager();
  EXPECT_EQ(1u, rm->GetNumAgents());

  remove(ROOTFILE);
}

}  // namespace bdm

#endif  // USE_DICT
