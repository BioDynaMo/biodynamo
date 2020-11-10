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
#include <experimental/filesystem>
#include <fstream>
#include <type_traits>

#include "core/agent/cell.h"
#include "core/resource_manager.h"
#include "core/simulation_backup.h"
#include "core/util/io.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_util.h"

namespace fs = std::experimental::filesystem;

namespace bdm {

class SimulationTest : public ::testing::Test {
 public:
  static constexpr const char* kTomlFileName = "bdm.toml";
  static constexpr const char* kTomlContent =
      "[simulation]\n"
      "random_seed = 123\n"
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
      "insitu = false\n"
      "export = true\n"
      "pv_insitu_pipeline = \"my-insitu-script.py\"\n"
      "pv_insitu_pipelinearguments = \"--param1=123\"\n"
      "interval = 100\n"
      "export_generate_pvsm = false\n"
      "compress_pv_files = false\n"
      "\n"
      "  [[visualize_agent]]\n"
      "  name = \"Cell\"\n"
      "\n"
      "  [[visualize_agent]]\n"
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
      "detect_static_agents = true\n"
      "cache_neighbors = true\n"
      "agent_uid_defragmentation_low_watermark = 0.123\n"
      "agent_uid_defragmentation_high_watermark = 0.456\n"
      "use_bdm_mem_mgr = false\n"
      "mem_mgr_aligned_pages_shift = 7\n"
      "mem_mgr_growth_rate = 1.123\n"
      "mem_mgr_max_mem_per_thread = 987654\n"
      "minimize_memory_while_rebalancing = false\n"
      "mapped_data_array_mode = \"cache\"\n"
      "\n"
      "[development]\n"
      "# this is a comment\n"
      "statistics = false\n"
      "debug_numa = true\n";

 protected:
  void SetUp() override {
    remove(kTomlFileName);
    remove("restore.root");
    CreateEmptyRestoreFile("restore.root");
    Simulation::counter_ = 0;
  }

  void TearDown() override {
    remove(kTomlFileName);
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
    EXPECT_EQ(123u, param->random_seed);
    EXPECT_EQ("paraview", param->visualization_engine);
    EXPECT_EQ("result-dir", param->output_dir);
    EXPECT_EQ("RK", param->diffusion_type);
    EXPECT_EQ(3600u, param->backup_interval);
    EXPECT_EQ(0.0125, param->simulation_time_step);
    EXPECT_EQ(2.0, param->simulation_max_displacement);
    EXPECT_FALSE(param->run_mechanical_interactions);
    EXPECT_TRUE(param->bound_space);
    EXPECT_EQ(-100, param->min_bound);
    EXPECT_EQ(200, param->max_bound);
    EXPECT_EQ(Param::ThreadSafetyMechanism::kAutomatic,
              param->thread_safety_mechanism);
    EXPECT_FALSE(param->insitu_visualization);
    EXPECT_TRUE(param->export_visualization);
    EXPECT_EQ("my-insitu-script.py", param->pv_insitu_pipeline);
    EXPECT_EQ("--param1=123", param->pv_insitu_pipelinearguments);
    EXPECT_EQ(100u, param->visualization_interval);
    EXPECT_FALSE(param->visualization_export_generate_pvsm);
    EXPECT_FALSE(param->visualization_compress_pv_files);

    // visualize_agent
    EXPECT_EQ(2u, param->visualize_agents.size());
    auto it = param->visualize_agents.cbegin();
    uint64_t counter = 0;
    while (it != param->visualize_agents.cend()) {
      if (counter == 0) {
        EXPECT_EQ("Cell", (*it).first);
        EXPECT_EQ(0u, (*it).second.size());
      } else if (counter == 1) {
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
    EXPECT_EQ(2u, param->visualize_diffusion.size());
    for (uint64_t i = 0; i < 2; i++) {
      auto vd = param->visualize_diffusion[i];
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
    EXPECT_EQ(123u, param->scheduling_batch_size);
    EXPECT_TRUE(param->detect_static_agents);
    EXPECT_TRUE(param->cache_neighbors);
    EXPECT_NEAR(0.123, param->agent_uid_defragmentation_low_watermark,
                abs_error<double>::value);
    EXPECT_NEAR(0.456, param->agent_uid_defragmentation_high_watermark,
                abs_error<double>::value);
    EXPECT_FALSE(param->use_bdm_mem_mgr);
    EXPECT_EQ(7u, param->mem_mgr_aligned_pages_shift);
    EXPECT_NEAR(1.123, param->mem_mgr_growth_rate, abs_error<double>::value);
    EXPECT_EQ(987654u, param->mem_mgr_max_mem_per_thread);
    EXPECT_FALSE(param->minimize_memory_while_rebalancing);
    EXPECT_EQ(Param::MappedDataArrayMode::kCache,
              param->mapped_data_array_mode);

    // development group
    EXPECT_FALSE(param->statistics);
    EXPECT_TRUE(param->debug_numa);
  }
};

#ifdef USE_DICT
TEST_F(SimulationTest, InitializeRuntimeParams) {
  std::ofstream config_file(kTomlFileName);
  config_file << kTomlContent;
  config_file.close();

  const char* argv[1] = {"./binary_name"};
  Simulation simulation(1, argv);
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file);
  EXPECT_EQ("restore.root", param->restore_file);
  EXPECT_EQ("binary_name", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
}

TEST_F(SimulationTest, InitializeRuntimeParams2) {
  std::ofstream config_file(kTomlFileName);
  config_file << kTomlContent;
  config_file.close();

  Simulation simulation("my-simulation");
  auto* param = simulation.GetParam();

  EXPECT_EQ("backup.root", param->backup_file);
  EXPECT_EQ("restore.root", param->restore_file);
  EXPECT_EQ("my-simulation", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
}

TEST_F(SimulationTest, InitializeRuntimeParamsWithCLIArguments) {
  std::ofstream config_file(kTomlFileName);
  config_file << kTomlContent;
  config_file.close();

  CreateEmptyRestoreFile("myrestore.root");
  const char* argv[5] = {"./binary_name", "-b", "mybackup.root", "-r",
                         "myrestore.root"};
  Simulation simulation(5, argv);
  auto* param = simulation.GetParam();

  // the following two parameters should contain the values from the command
  // line arguments.
  EXPECT_EQ("mybackup.root", param->backup_file);
  EXPECT_EQ("myrestore.root", param->restore_file);
  EXPECT_EQ("binary_name", simulation.GetUniqueName());
  ValidateNonCLIParameter(param);
  remove("myrestore.root");
}

TEST_F(SimulationTest, InitializeRuntimeParamsCLIConfigFileName) {
  std::string config_filename = "my-config-file.toml";
  remove(kTomlFileName);
  remove(config_filename.c_str());
  std::ofstream config_file(config_filename);
  config_file << kTomlContent;
  config_file.close();

  const char* argv[3] = {"./binary_name", "-c", config_filename.c_str()};
  Simulation simulation(3, argv);

  ValidateNonCLIParameter(simulation.GetParam());
  remove("myrestore.root");
  remove(config_filename.c_str());
}

TEST_F(SimulationTest, InitializeRuntimeParamsCtorConfigFileName) {
  std::string config_filename = "my-config-file.toml";
  remove(kTomlFileName);
  remove(config_filename.c_str());
  std::ofstream config_file(config_filename);
  config_file << kTomlContent;
  config_file.close();

  {
    const char* argv[1] = {"./binary_name"};
    Simulation simulation(1, argv, {config_filename});
    ValidateNonCLIParameter(simulation.GetParam());
  }

  {
    Simulation simulation("./binary_name", {config_filename});
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

TEST_F(SimulationTest, MultipleJsonConfigsAndPrecedence) {
  const char* ctor1_config = R"EOF(
{
  "bdm::Param": {
    "random_seed": 1,
    "scheduling_batch_size": 1,
    "backup_file": "ctor1",
    "mem_mgr_growth_rate": 1.11,
    "backup_interval": 1,
    "simulation_time_step": 1
  }
}
)EOF";

  // overwrite all but first parameter
  const char* ctor2_config = R"EOF(
{
  "bdm::Param": {
    "scheduling_batch_size": 2,
    "backup_file": "ctor2",
    "mem_mgr_growth_rate": 1.12,
    "backup_interval": 2,
    "simulation_time_step": 2
  }
}
)EOF";

  // overwrite all but first two parameter
  const char* cli1_config = R"EOF(
{
  "bdm::Param": {
    "backup_file": "cli1",
    "mem_mgr_growth_rate": 1.13,
    "backup_interval": 3,
    "simulation_time_step": 3
  }
}
)EOF";

  // overwrite all but first three parameter
  const char* cli2_config = R"EOF(
{
  "bdm::Param": {
    "mem_mgr_growth_rate": 1.14,
    "backup_interval": 4,
    "simulation_time_step": 4
  }
}
)EOF";

  WriteToFile("ctor1.json", ctor1_config);
  WriteToFile("ctor2.json", ctor2_config);
  WriteToFile("cli1.json", cli1_config);
  WriteToFile("cli2.json", cli2_config);

  const char* argv[9] = {TEST_NAME,
                         "-c",
                         "cli1.json",
                         "-c",
                         "cli2.json",
                         "--inline-config",
                         "{ \"bdm::Param\": { \"backup_interval\": 5, "
                         "\"simulation_time_step\": 5 }}",
                         "--inline-config",
                         "{ \"bdm::Param\": { \"simulation_time_step\": 6 }}"};

  Simulation sim(9, argv, {"ctor1.json", "ctor2.json"});
  auto* param = sim.GetParam();

  EXPECT_EQ(1u, param->random_seed);
  EXPECT_EQ(2u, param->scheduling_batch_size);
  EXPECT_EQ("cli1", param->backup_file);
  EXPECT_NEAR(1.14, param->mem_mgr_growth_rate, abs_error<double>::value);
  EXPECT_EQ(5u, param->backup_interval);
  EXPECT_NEAR(6.0, param->simulation_time_step, abs_error<double>::value);

  std::remove("ctor1.json");
  std::remove("ctor2.json");
  std::remove("cli1.json");
  std::remove("cli2.json");
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

TEST_F(SimulationTest, InlineConfig) {
  const char* argv[3] = {
      "./binary_name", "--inline-config",
      "{ \"bdm::Param\": { \"simulation_time_step\": 6.28}}"};
  Simulation sim(3, argv);
  EXPECT_NEAR(6.28, sim.GetParam()->simulation_time_step, 1e-5);
}

TEST_F(SimulationTest, DontRemoveOutputDirContents) {
  fs::create_directory(Concat("output/", TEST_NAME));
  fs::create_directory(Concat("output/", TEST_NAME, "/subdir"));
  EXPECT_FALSE(fs::is_empty(Concat("output/", TEST_NAME)));

  Simulation sim(TEST_NAME);
  EXPECT_FALSE(fs::is_empty(Concat("output/", TEST_NAME)));
}

TEST_F(SimulationTest, RemoveOutputDirContents) {
  fs::create_directory(Concat("output/", TEST_NAME));
  fs::create_directory(Concat("output/", TEST_NAME, "/subdir"));
  EXPECT_FALSE(fs::is_empty(Concat("output/", TEST_NAME)));

  auto set_param = [](Param* param) {
    param->remove_output_dir_contents = true;
  };
  Simulation sim(TEST_NAME, set_param);
  EXPECT_TRUE(fs::is_empty(Concat("output/", TEST_NAME)));
}

#ifdef USE_DICT
TEST_F(IOTest, Simulation) {
  // change state of each data member in Simulation

  auto set_param = [](Param* param) { param->simulation_time_step = 3.14; };
  Simulation sim(TEST_NAME, set_param);
  auto* rm = sim.GetResourceManager();
  auto* param = sim.GetParam();
  rm->AddAgent(new Cell());
  rm->AddAgent(new Cell());
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    r->SetSeed(42);
    r->Uniform(12, 34);
  }

  Simulation* restored;
  BackupAndRestore(sim, &restored);
  EXPECT_EQ(2u, restored->GetResourceManager()->GetNumAgents());

  // store next random number for later comparison
  std::vector<double> next_rand;
  next_rand.resize(omp_get_max_threads());
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    next_rand[omp_get_thread_num()] = r->Uniform(12, 34);
  }

  // change state to see if call to Simulation::Restore was successful
  rm->ClearAgents();
  const_cast<Param*>(param)->simulation_time_step = 6.28;
  // check if rm is really empty to avoid false positive test results
  EXPECT_EQ(0u, rm->GetNumAgents());

  // assign restored simulation to current one
  sim.Restore(std::move(*restored));
  delete restored;

  // Validate results;
  // From each data member in simulation do one check
  // For more detailed iotest see the repective classes
  // rm and param should still be valid!
  const double kEpsilon = abs_error<double>::value;
  EXPECT_EQ(2u, rm->GetNumAgents());
  EXPECT_NEAR(3.14, param->simulation_time_step, kEpsilon);
#pragma omp parallel
  {
    auto* r = sim.GetRandom();
    EXPECT_NEAR(next_rand[omp_get_thread_num()], r->Uniform(12, 34), kEpsilon);
  }
}

// The Param IOTest is located here to reuse the infrastructure used to test
// parsing parameters.
TEST_F(SimulationTest, ParamIOTest) {
  std::ofstream config_file(kTomlFileName);
  config_file << kTomlContent;
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
  // NB visualize_agents is currently not backed up due to a ROOT error
  restored->visualize_agents = param->visualize_agents;

  ValidateNonCLIParameter(restored);
  remove(root_file);
  delete restored;
}

#endif  // USE_DICT

}  // namespace bdm
