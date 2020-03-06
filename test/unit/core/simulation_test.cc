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

#include <gtest/gtest.h>
#include <omp.h>
#include <fstream>
#include <type_traits>

#include "core/resource_manager.h"
#include "core/sim_object/cell.h"
#include "core/simulation_backup.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_util.h"

namespace bdm {

class SimulationTest : public ::testing::Test {
 public:
  static constexpr const char* kConfigFileName = "bdm.toml";
  static constexpr const char* kConfigContent =
      "[simulation]\n"
      "output_dir = \"result-dir\"\n"
      "backup_file = \"backup.root\"\n"
      "restore_file = \"restore.root\"\n"
      "backup_interval = 3600\n"
      "time_step = 0.0125\n"
      "max_displacement = 2.0\n"
      "run_mechanical_interactions = false\n"
      "bound_space = true\n"
      "min_bound = -100\n"
      "max_bound =  200\n"
      "diffusion_type = \"RK\"\n"
      "thread_safety_mechanism = \"automatic\"\n"
      "\n"
      "[visualization]\n"
      "live = false\n"
      "export = true\n"
      "python_paraview_pipeline = \"my-insitu-script.py\"\n"
      "export_interval = 100\n"
      "export_generate_pvsm = false\n"
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
      "[performance]\n"
      "scheduling_batch_size = 123\n"
      "detect_static_sim_objects = true\n"
      "cache_neighbors = true\n"
      "souid_defragmentation_low_watermark = 0.123\n"
      "souid_defragmentation_high_watermark = 0.456\n"
      "use_bdm_mem_mgr = false\n"
      "mem_mgr_aligned_pages_shift = 7\n"
      "mem_mgr_growth_rate = 1.123\n"
      "mem_mgr_max_mem_per_thread = 987654\n"
      "minimize_memory_while_rebalancing = false\n"
      "\n"
      "[development]\n"
      "# this is a comment\n"
      "statistics = true\n"
      "debug_numa = true\n";

 protected:
  virtual void SetUp() {
    remove(kConfigFileName);
    remove("restore.root");
    CreateEmptyRestoreFile("restore.root");
    Simulation::counter_ = 0;
  }

  virtual void TearDown() {
    remove(kConfigFileName);
    remove("restore.root");
  }

  /// Creates an empty file restore file. \n
  /// It is needed, because BioDynaMo throws a fatal exception if it is
  /// initialized with a restore file that does not exist.
  void CreateEmptyRestoreFile(const std::string& filename) {
    Simulation sim("CreateEmptyRestoreFile");
    SimulationBackup b(filename, "");
    b.Backup(0);
    Simulation::counter_ = 0;
  }

  void ValidateNonCLIParameter(const Param* param) {
    EXPECT_EQ("paraview", param->visualization_engine_);
    EXPECT_EQ("result-dir", param->output_dir_);
    EXPECT_EQ("RK", param->diffusion_type_);
    EXPECT_EQ(3600u, param->backup_interval_);
    EXPECT_EQ(0.0125, param->simulation_time_step_);
    EXPECT_EQ(2.0, param->simulation_max_displacement_);
    EXPECT_FALSE(param->run_mechanical_interactions_);
    EXPECT_TRUE(param->bound_space_);
    EXPECT_EQ(-100, param->min_bound_);
    EXPECT_EQ(200, param->max_bound_);
    EXPECT_EQ(Param::ThreadSafetyMechanism::kAutomatic,
              param->thread_safety_mechanism_);
    EXPECT_FALSE(param->live_visualization_);
    EXPECT_TRUE(param->export_visualization_);
    EXPECT_EQ("my-insitu-script.py", param->python_paraview_pipeline_);
    EXPECT_EQ(100u, param->visualization_export_interval_);
    EXPECT_FALSE(param->visualization_export_generate_pvsm_);

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

    // performance group
    EXPECT_EQ(123u, param->scheduling_batch_size_);
    EXPECT_TRUE(param->detect_static_sim_objects_);
    EXPECT_TRUE(param->cache_neighbors_);
    EXPECT_NEAR(0.123, param->souid_defragmentation_low_watermark_,
                abs_error<double>::value);
    EXPECT_NEAR(0.456, param->souid_defragmentation_high_watermark_,
                abs_error<double>::value);
    EXPECT_FALSE(param->use_bdm_mem_mgr_);
    EXPECT_EQ(7u, param->mem_mgr_aligned_pages_shift_);
    EXPECT_NEAR(1.123, param->mem_mgr_growth_rate_, abs_error<double>::value);
    EXPECT_EQ(987654u, param->mem_mgr_max_mem_per_thread_);
    EXPECT_FALSE(param->minimize_memory_while_rebalancing_);

    // development group
    EXPECT_TRUE(param->statistics_);
    EXPECT_TRUE(param->debug_numa_);
  }
};

#ifdef USE_DICT
TEST_F(SimulationTest, InitializeRuntimeParams) {
  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  const char* argv[1] = {"./binary_name"};
  Simulation simulation(1, argv);
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file_);
  EXPECT_EQ("restore.root", param->restore_file_);
  EXPECT_EQ("binary_name", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
}

TEST_F(SimulationTest, InitializeRuntimeParams2) {
  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  Simulation simulation("my-simulation");
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file_);
  EXPECT_EQ("restore.root", param->restore_file_);
  EXPECT_EQ("my-simulation", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
}

TEST_F(SimulationTest, InitializeRuntimeParamsWithCLIArguments) {
  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  CreateEmptyRestoreFile("myrestore.root");
  const char* argv[5] = {"./binary_name", "-b", "mybackup.root", "-r",
                         "myrestore.root"};
  Simulation simulation(5, argv);
  auto* param = simulation.GetParam();

  // the following two parameters should contain the values from the command
  // line arguments.
  EXPECT_EQ("mybackup.root", param->backup_file_);
  EXPECT_EQ("myrestore.root", param->restore_file_);
  EXPECT_EQ("binary_name", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
  remove("myrestore.root");
}

TEST_F(SimulationTest, InitializeRuntimeParamsCLIConfigFileName) {
  std::string config_filename = "my-config-file.toml";
  remove(kConfigFileName);
  remove(config_filename.c_str());
  std::ofstream config_file(config_filename);
  config_file << kConfigContent;
  config_file.close();

  const char* argv[3] = {"./binary_name", "-c", config_filename.c_str()};
  Simulation simulation(3, argv);

  ValidateNonCLIParameter(simulation.GetParam());
  remove("myrestore.root");
  remove(config_filename.c_str());
}

TEST_F(SimulationTest, InitializeRuntimeParamsCtorConfigFileName) {
  std::string config_filename = "my-config-file.toml";
  remove(kConfigFileName);
  remove(config_filename.c_str());
  std::ofstream config_file(config_filename);
  config_file << kConfigContent;
  config_file.close();

  {
    const char* argv[1] = {"./binary_name"};
    Simulation simulation(1, argv, config_filename);
    ValidateNonCLIParameter(simulation.GetParam());
  }

  {
    Simulation simulation("./binary_name", config_filename);
    ValidateNonCLIParameter(simulation.GetParam());
  }

  // test presedence of ctor config_file over cli config file
  {
    const char* argv[3] = {"./binary_name", "-c", "does-not-exist.toml"};
    Simulation simulation(3, argv, config_filename);
    ValidateNonCLIParameter(simulation.GetParam());
  }

  remove("myrestore.root");
  remove(config_filename.c_str());
}

TEST_F(SimulationTest, InitializeRuntimeParamsSimulationName) {
  // same working dir
  const char* argv0[1] = {"./binary_name"};
  Simulation simulation0(1, argv0);
  EXPECT_EQ("binary_name", simulation0.GetUniqueName());

  // in PATH
  const char* argv1[1] = {"binary_name"};
  Simulation simulation1(1, argv1);
  EXPECT_EQ("binary_name1", simulation1.GetUniqueName());

  // binary dir != working dir
  const char* argv2[1] = {"./build/binary_name"};
  Simulation simulation2(1, argv2);
  EXPECT_EQ("binary_name2", simulation2.GetUniqueName());

  Simulation simulation3("binary_name");
  EXPECT_EQ("binary_name3", simulation3.GetUniqueName());
}

#endif  // USE_DICT

TEST_F(SimulationTest, SimulationId_OutputDir) {
  Simulation simulation("my-simulation");
  Simulation simulation1("my-simulation");

  EXPECT_EQ("my-simulation", simulation.GetUniqueName());
  EXPECT_EQ("output/my-simulation", simulation.GetOutputDir());

  EXPECT_EQ("my-simulation1", simulation1.GetUniqueName());
  EXPECT_EQ("output/my-simulation1", simulation1.GetOutputDir());
}

TEST_F(SimulationTest, SimulationId_OutputDir2) {
  Simulation simulation("");

  EXPECT_EQ("", simulation.GetUniqueName());
  EXPECT_EQ("output", simulation.GetOutputDir());
}

#ifdef USE_DICT
TEST_F(IOTest, Simulation) {
  // change state of each data member in Simulation

  auto set_param = [](Param* param) { param->simulation_time_step_ = 3.14; };
  Simulation sim(TEST_NAME, set_param);
  auto* rm = sim.GetResourceManager();
  auto* param = sim.GetParam();
  rm->push_back(new Cell());
  rm->push_back(new Cell());
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    r->SetSeed(42);
    r->Uniform(12, 34);
  }

  Simulation* restored;
  BackupAndRestore(sim, &restored);
  EXPECT_EQ(2u, restored->GetResourceManager()->GetNumSimObjects());

  // store next random number for later comparison
  std::vector<double> next_rand;
  next_rand.resize(omp_get_max_threads());
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    next_rand[omp_get_thread_num()] = r->Uniform(12, 34);
  }

  // change state to see if call to Simulation::Restore was successful
  rm->Clear();
  const_cast<Param*>(param)->simulation_time_step_ = 6.28;
  // check if rm is really empty to avoid false positive test results
  EXPECT_EQ(0u, rm->GetNumSimObjects());

  // assign restored simulation to current one
  sim.Restore(std::move(*restored));
  delete restored;

  // Validate results;
  // From each data member in simulation do one check
  // For more detailed iotest see the repective classes
  // rm and param should still be valid!
  const double kEpsilon = abs_error<double>::value;
  EXPECT_EQ(2u, rm->GetNumSimObjects());
  EXPECT_NEAR(3.14, param->simulation_time_step_, kEpsilon);
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    EXPECT_NEAR(next_rand[omp_get_thread_num()], r->Uniform(12, 34), kEpsilon);
  }
}

// The Param IOTest is located here to reuse the infrastructure used to test
// parsing parameters.
TEST_F(SimulationTest, ParamIOTest) {
  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  Simulation simulation(TEST_NAME);
  auto* param = simulation.GetParam();

  Param* restored;
  BackupAndRestore(*param, &restored);
  const char* root_file = "param.root";
  remove(root_file);
  // write to root file
  WritePersistentObject(root_file, "param", *param, "new");

  // read back
  GetPersistentObject(root_file, "param", restored);
  // NB visualize_sim_objects_ is currently not backed up due to a ROOT error
  restored->visualize_sim_objects_ = param->visualize_sim_objects_;

  ValidateNonCLIParameter(restored);
  remove(root_file);
  delete restored;
}

#endif  // USE_DICT

}  // namespace bdm
