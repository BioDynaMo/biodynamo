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

#include "bdm_imp.h"
#include "unit/default_ctparam.h"

#include <fstream>
#include "gtest/gtest.h"

namespace bdm {



class BdmSimTest : public ::testing::Test {
 public:
   static constexpr char* gConfigFileName = "bdm.toml";
   static constexpr char* gConfigContent =
       "[simulation]\n"
       "backup_file = \"backup.root\"\n"
       "restore_file = \"restore.root\"\n"
       "backup_interval = 3600\n"
       "time_step = 0.0125\n"
       "max_displacement = 2.0\n"
       "run_mechanical_interactions = false\n"
       "bound_space = true\n"
       "min_bound = -100\n"
       "max_bound =  200\n"
       "\n"
       "[visualization]\n"
       "live = true\n"
       "export = true\n"
       "export_interval = 100\n"
       "\n"
       "  [[visualize_sim_object]]\n"
       "  name = \"Cell\"\n"
       "\n"
       "  [[visualize_sim_object]]\n"
       "  name = \"Neurite\"\n"
       "  additional_data_members = [ \"spring_axis_\", \"tension_\" ]\n"
       "\n"
       "\n"
       "  [[visualize_diffusion]]\n"
       "  name = \"Na\"\n"
       "  concentration = false\n"
       "  gradient = true\n"
       "\n"
       "  [[visualize_diffusion]]\n"
       "  name = \"K\"\n"
       "\n"
       "[development]\n"
       "# this is a comment\n"
       "statistics = true\n";

 protected:
  virtual void SetUp() {
    remove(gConfigFileName);
    remove("restore.root");
    CreateEmptyFile("restore.root");
  }

  virtual void TearDown() {
    remove(gConfigFileName);
  }

  /// Creates an empty file. \n
  /// Needed, because BioDynaMo throws an fatal exception if it is initialized
  /// with a restore file that does not exist.
  void CreateEmptyFile(const std::string& filename) {
    // std::fstream s(filename, s.binary | s.out);
    // if (!s.is_open()) {
    //   std::cout << "failed to open " << filename << '\n';
    // } else {
    //   s << "";
    // }
    SimulationBackup b(filename, "");
    b.Backup(0);
  }

  void ValidateNonCLIParameter(Param* param) {
    EXPECT_EQ(3600u, param->backup_interval_);
    EXPECT_EQ(0.0125, param->simulation_time_step_);
    EXPECT_EQ(2.0, param->simulation_max_displacement_);
    EXPECT_FALSE(param->run_mechanical_interactions_);
    EXPECT_TRUE(param->bound_space_);
    EXPECT_EQ(-100, param->min_bound_);
    EXPECT_EQ(200, param->max_bound_);
    EXPECT_TRUE(param->live_visualization_);
    EXPECT_TRUE(param->export_visualization_);
    EXPECT_EQ(100u, param->visualization_export_interval_);

    // visualize_sim_object
    EXPECT_EQ(2u, param->visualize_sim_objects_.size());
    auto it = param->visualize_sim_objects_.cbegin();
    uint64_t counter = 0;
    while (it != param->visualize_sim_objects_.cend()) {
      if (counter == 1) {
        EXPECT_EQ("Cell", (*it).first);
        EXPECT_EQ(0u, (*it).second.size());
      } else if (counter == 0) {
        EXPECT_EQ("Neurite", (*it).first);
        auto additional_dm = (*it).second;
        EXPECT_EQ(2u, additional_dm.size());
        EXPECT_TRUE(additional_dm.find("spring_axis_") != additional_dm.end());
        EXPECT_TRUE(additional_dm.find("tension_") != additional_dm.end());
      }
      counter++;
      it++;
    }

    // visualize_diffusion
    EXPECT_EQ(2u, param->visualize_diffusion_.size());
    for (uint64_t i = 0; i < 2; i++) {
      auto vd = param->visualize_diffusion_[i];
      if (i == 0) {
        EXPECT_EQ("Na", vd.name_);
        EXPECT_FALSE(vd.concentration_);
        EXPECT_TRUE(vd.gradient_);
      } else if (i == 1) {
        EXPECT_EQ("K", vd.name_);
        EXPECT_TRUE(vd.concentration_);
        EXPECT_FALSE(vd.gradient_);
      }
    }

    EXPECT_TRUE(param->statistics_);
  }
};



TEST_F(BdmSimTest, InitializeRuntimeParams) {
  std::ofstream config_file(gConfigFileName);
  config_file << gConfigContent;
  config_file.close();

  const char* argv[1] = {"./binary_name"};
  BdmSim<> simulation(1, argv);
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file_);
  EXPECT_EQ("restore.root", param->restore_file_);
  EXPECT_EQ("binary_name", simulation.GetSimulationId());
  ValidateNonCLIParameter(param);
}

TEST_F(BdmSimTest, InitializeRuntimeParams2) {
  std::ofstream config_file(gConfigFileName);
  config_file << gConfigContent;
  config_file.close();

  BdmSim<> simulation("my-simulation");
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file_);
  EXPECT_EQ("restore.root", param->restore_file_);
  EXPECT_EQ("my-simulation", simulation.GetSimulationId());
  ValidateNonCLIParameter(param);
}

TEST_F(BdmSimTest, InitializeRuntimeParamsWithCLIArguments) {
  std::ofstream config_file(gConfigFileName);
  config_file << gConfigContent;
  config_file.close();

  CreateEmptyFile("myrestore.root");
  const char* argv[5] = {"./binary_name", "-b", "mybackup.root", "-r",
                         "myrestore.root"};
  BdmSim<> simulation(5, argv);
  auto* param = simulation.GetParam();

  // the following two parameters should contain the values from the command
  // line arguments.
  EXPECT_EQ("mybackup.root", param->backup_file_);
  EXPECT_EQ("myrestore.root", param->restore_file_);
  EXPECT_EQ("binary_name", simulation.GetSimulationId());
  ValidateNonCLIParameter(param);
  remove("myrestore.root");
}

TEST_F(BdmSimTest, InitializeRuntimeParamsSimulationName) {
  // same working dir
  const char* argv0[1] = {"./binary_name"};
  BdmSim<> simulation0(1, argv0);
  EXPECT_EQ("binary_name", simulation0.GetSimulationId());

  // in PATH
  const char* argv1[1] = {"binary_name"};
  BdmSim<> simulation1(1, argv1);
  EXPECT_EQ("binary_name", simulation1.GetSimulationId());

  // binary dir != working dir
  const char* argv2[1] = {"./build/binary_name"};
  BdmSim<> simulation2(1, argv2);
  EXPECT_EQ("binary_name", simulation2.GetSimulationId());

  BdmSim<> simulation3("binary_name");
  EXPECT_EQ("binary_name", simulation3.GetSimulationId());
}

TEST_F(BdmSimTest, SimulationId) {
  BdmSim<> simulation("my-simulation");
  BdmSim<> simulation1("my-simulation");

  EXPECT_EQ("my-simulation", simulation.GetSimulationId());
  EXPECT_EQ("my-simulation1", simulation1.GetSimulationId());
}

}  // namespace bdm
