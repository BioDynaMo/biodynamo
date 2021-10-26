// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------
#ifdef USE_PARAVIEW

#include <gtest/gtest.h>
#include <experimental/filesystem>

#include "biodynamo.h"
#include "core/visualization/visualization_adaptor.h"
#include "neuroscience/neuroscience.h"
#include "unit/test_util/test_util.h"

namespace fs = std::experimental::filesystem;

namespace bdm {

// -----------------------------------------------------------------------------
std::string GetPythonScriptPath(const std::string& python_script) {
  std::stringstream path;
  path << std::getenv("BDMSYS") << "/share/test/core/visualization/paraview/"
       << python_script;
  return path.str();
}

// -----------------------------------------------------------------------------
void Validate(const std::string& python_script, const std::string& sim_name,
              uint64_t num_elements, bool use_pvsm) {
  std::stringstream cmd;
  std::string pv_dir = std::getenv("ParaView_DIR");

  cmd << pv_dir << "/bin/pvbatch " << GetPythonScriptPath(python_script)
      << " --sim_name=" << sim_name << " --num_elements=" << num_elements;
  if (use_pvsm) {
    cmd << " --use_pvsm";
  }
  int ret_code = system(cmd.str().c_str());
  EXPECT_EQ(0, ret_code);
}

// -----------------------------------------------------------------------------
/// For insitu visualization tests, this function is called in a different
/// process.
/// Therefore, it uses exit(0) at the end to indicate a passing test.
/// All ASSERT* macros exit the function before the macro if they evaluate
/// to false, thus failing the test also in the insitu case
void RunDiffusionGridTest(uint64_t max_bound, uint64_t resolution,
                          bool export_visualization = true,
                          bool use_pvsm = true) {
  auto num_diffusion_boxes = std::pow(resolution, 3);
  auto set_param = [&](Param* param) {
    param->remove_output_dir_contents = true;
    param->min_bound = 0;
    param->max_bound = max_bound;
    param->export_visualization = export_visualization;
    param->insitu_visualization = !export_visualization;
    if (!export_visualization) {
      param->pv_insitu_pipeline =
          GetPythonScriptPath("validate_diffusion_grid.py");
      auto sim_name = Simulation::GetActive()->GetUniqueName();
      param->pv_insitu_pipelinearguments = Concat(
          "--sim_name=", sim_name, " --num_elements=", num_diffusion_boxes);
    }
    param->visualize_diffusion.push_back({"Substance", true});
  };
  auto sim_name =
      Concat("ExportDiffusionGridTest_", max_bound, "_", resolution);
  auto* sim = new Simulation(sim_name, set_param);
  auto output_dir = sim->GetOutputDir();

  ModelInitializer::DefineSubstance(0, "Substance", 0.0000000001, 0.000000001,
                                    resolution);
  // create a sequence 1, 2, 3...
  // since initialization is multithreaded returning in increasing counter
  // does not work. -> calculate and return box id
  ModelInitializer::InitializeSubstance(0, [&](double x, double y, double z) {
    auto* dg =
        Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(0);
    auto grid_dimensions = dg->GetDimensions();
    auto box_length = dg->GetBoxLength();

    std::array<uint32_t, 3> box_coord;
    box_coord[0] = round((x - grid_dimensions[0]) / box_length);
    box_coord[1] = round((y - grid_dimensions[2]) / box_length);
    box_coord[2] = round((z - grid_dimensions[4]) / box_length);
    const auto& num_boxes = dg->GetNumBoxesArray();
    return box_coord[2] * num_boxes[0] * num_boxes[1] +
           box_coord[1] * num_boxes[0] + box_coord[0];
  });

  // every simulation needs at least one agent
  auto* cell = new Cell();
  cell->SetDiameter(10);
  cell->SetPosition({5, 5, 5});
  sim->GetResourceManager()->AddAgent(cell);
  auto* cell1 = new Cell();
  cell1->SetDiameter(10);
  double pos = static_cast<double>(max_bound) - 5;
  cell1->SetPosition({pos, pos, pos});
  sim->GetResourceManager()->AddAgent(cell1);

  sim->GetScheduler()->Simulate(1);

  auto* dg = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(0);
  EXPECT_EQ(num_diffusion_boxes, dg->GetNumBoxes());
  sim_name = sim->GetUniqueName();
  // create pvsm file
  delete sim;

  // NB: for insitu visualization the validation step happened in call Simulate
  if (export_visualization) {
    Validate("validate_diffusion_grid.py", sim_name, num_diffusion_boxes,
             use_pvsm);
  }
  ASSERT_TRUE(fs::exists(Concat(output_dir, "/valid")));
  // Test will run in separate process.
  exit(0);
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportDiffusionGrid_SlicesLtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(RunDiffusionGridTest(std::max(max_threads - 1, 1),
                                             std::max(max_threads - 1, 1)));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportDiffusionGrid_SlicesGtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(RunDiffusionGridTest(3 * max_threads + 1, max_threads));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportDiffusionGridLoadWithoutPVSM) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(RunDiffusionGridTest(
      std::max(max_threads - 1, 1), std::max(max_threads - 1, 1), true, false));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, InsituDiffusionGrid_SlicesLtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(RunDiffusionGridTest(
      std::max(max_threads - 1, 1), std::max(max_threads - 1, 1), false));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, InsituDiffusionGrid_SlicesGtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(
      RunDiffusionGridTest(3 * max_threads + 1, max_threads, false));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void RunAgentsTest(Param::MappedDataArrayMode mode, uint64_t num_agents,
                   bool export_visualization = true, bool use_pvsm = true) {
  auto set_param = [&](Param* param) {
    param->remove_output_dir_contents = true;
    param->export_visualization = export_visualization;
    param->insitu_visualization = !export_visualization;
    param->visualization_export_generate_pvsm = use_pvsm;
    if (!export_visualization) {
      param->pv_insitu_pipeline = GetPythonScriptPath("validate_agents.py");
      auto sim_name = Simulation::GetActive()->GetUniqueName();
      param->pv_insitu_pipelinearguments =
          Concat("--sim_name=", sim_name, " --num_elements=", num_agents);
    }
    param->unschedule_default_operations = {"mechanical forces"};
    param->visualize_agents.insert(
        {"NeuriteElement", {"uid_", "daughter_right_"}});
    param->mapped_data_array_mode = mode;
  };
  neuroscience::InitModule();
  auto sim_name = Concat("ExportAgentsTest_", num_agents, "_", mode);
  auto* sim = new Simulation(sim_name, set_param);

  auto output_dir = sim->GetOutputDir();
  fs::remove_all(output_dir);
  fs::create_directory(output_dir);

  using NeuriteElement = neuroscience::NeuriteElement;

  auto* rm = sim->GetResourceManager();

  auto construct = [&](uint64_t i) {
    auto* ne = new NeuriteElement();
    auto d = static_cast<double>(i);
    ne->SetDiameter(d + 10);
    ne->SetMassLocation({d, d, d});
    ne->SetActualLength(d + 10);
    ne->SetDaughterRight(AgentPointer<NeuriteElement>(AgentUid(i)));
    rm->AddAgent(ne);
  };

  for (uint64_t i = 0; i < num_agents; ++i) {
    construct(i);
  }

  // Don't run a simulation step, because neurites are not properly set up.
  (*sim->GetScheduler()->GetOps("visualize")[0])();

  sim_name = sim->GetUniqueName();

  // NB: for insitu visualization the validation step happened in call Simulate
  if (export_visualization) {
    // create pvsm file
    delete sim;
    Validate("validate_agents.py", sim_name, num_agents, use_pvsm);
    ASSERT_TRUE(fs::exists(Concat(output_dir, "/valid")));
  } else {
    delete sim;
    ASSERT_TRUE(fs::exists(Concat(output_dir, "/valid")));
  }
  // Test will run in separate process.
  exit(0);
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportAgents_ZeroCopy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, std::max(1, max_threads - 1)));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportAgents_Cache) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCache;
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, std::max(1, max_threads - 1)));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportAgents_Copy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCopy;
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, std::max(1, max_threads - 1)));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, ExportAgentsLoadWithoutPVSM) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  LAUNCH_IN_NEW_PROCESS(
      RunAgentsTest(mode, std::max(1, max_threads - 1), true, false));
}

// Disable insitu tests until ROOT cling crash on MacOS has been resolved
#ifndef __APPLE__
// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, InsituAgents_ZeroCopy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  LAUNCH_IN_NEW_PROCESS(
      RunAgentsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1, false));
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, InsituAgents_Cache) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCache;
  LAUNCH_IN_NEW_PROCESS(
      RunAgentsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1, false));
}

// -----------------------------------------------------------------------------
void RunDefaultInsituPipelineTest() {
  auto set_param = [](Param* param) {
    param->remove_output_dir_contents = true;
    param->insitu_visualization = true;
    param->visualize_agents.insert({"Cell", {}});
  };
  Simulation simulation("RunDefaultInsituPipelineTest", set_param);

  auto* rm = simulation.GetResourceManager();
  auto* cell = new Cell(30);
  rm->AddAgent(cell);

  simulation.GetScheduler()->Simulate(1);
  // Test passes if there is no crash
  // Test will run in separate process.
  exit(0);
}

TEST(FLAKY_ParaviewIntegrationTest, DefaultInsituPipeline) {
  LAUNCH_IN_NEW_PROCESS(RunDefaultInsituPipelineTest());
}

// -----------------------------------------------------------------------------
TEST(FLAKY_ParaviewIntegrationTest, InsituAgents_Copy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCopy;
  LAUNCH_IN_NEW_PROCESS(
      RunAgentsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunAgentsTest(mode, 10 * max_threads + 1, false));
}
#endif  // __APPLE__

}  // namespace bdm

#endif  // USE_PARAVIEW
