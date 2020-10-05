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
    param->min_bound_ = 0;
    param->max_bound_ = max_bound;
    param->export_visualization_ = export_visualization;
    param->insitu_visualization_ = !export_visualization;
    if (!export_visualization) {
      param->pv_insitu_pipeline_ =
          GetPythonScriptPath("validate_diffusion_grid.py");
      auto sim_name = Simulation::GetActive()->GetUniqueName();
      param->pv_insitu_pipeline_arguments_ = Concat(
          "--sim_name=", sim_name, " --num_elements=", num_diffusion_boxes);
    }
    param->visualize_diffusion_.push_back({"Substance", true});
  };
  auto sim_name =
      Concat("ExportDiffusionGridTest_", max_bound, "_", resolution);
  auto* sim = new Simulation(sim_name, set_param);
  auto output_dir = sim->GetOutputDir();
  // fs::remove_all(output_dir);
  // fs::create_directory(output_dir);

  ModelInitializer::DefineSubstance(0, "Substance", 0.0000000001, 0.000000001, resolution);
  // create a sequence 1, 2, 3...
  // since initialization is multithreaded returning in increasing counter
  // does not work. -> calculate and return box id
  ModelInitializer::InitializeSubstance(
      0, "Substance", [&](double x, double y, double z) {
        auto* dg =
            Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(0);
        auto grid_dimensions = dg->GetDimensions();
        auto box_length = dg->GetBoxLength();

        std::array<uint32_t, 3> box_coord;
        box_coord[0] = (floor(x) - grid_dimensions[0]) / box_length;
        box_coord[1] = (floor(y) - grid_dimensions[2]) / box_length;
        box_coord[2] = (floor(z) - grid_dimensions[4]) / box_length;

        auto& num_boxes = dg->GetNumBoxesArray();
        return box_coord[2] * num_boxes[0] * num_boxes[1] +
               box_coord[1] * num_boxes[0] + box_coord[0];
      });

  // every simulation needs at least one sim object
  auto* cell = new Cell();
  cell->SetDiameter(10);
  cell->SetPosition({5, 5, 5});
  sim->GetResourceManager()->push_back(cell);
  auto* cell1 = new Cell();
  cell1->SetDiameter(10);
  double pos = static_cast<double>(max_bound) - 5;
  cell1->SetPosition({pos, pos, pos});
  sim->GetResourceManager()->push_back(cell1);

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
  if (!export_visualization) {
    exit(0);
  }
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportDiffusionGrid_SlicesLtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  RunDiffusionGridTest(std::max(max_threads - 1, 1),
                       std::max(max_threads - 1, 1));
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportDiffusionGrid_SlicesGtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  RunDiffusionGridTest(3 * max_threads + 1, max_threads);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportDiffusionGridLoadWithoutPVSM) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  RunDiffusionGridTest(std::max(max_threads - 1, 1),
                       std::max(max_threads - 1, 1), true, false);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, InsituDiffusionGrid_SlicesLtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(RunDiffusionGridTest(
      std::max(max_threads - 1, 1), std::max(max_threads - 1, 1), false));
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, InsituDiffusionGrid_SlicesGtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  LAUNCH_IN_NEW_PROCESS(
      RunDiffusionGridTest(3 * max_threads + 1, max_threads, false));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void RunSimObjectsTest(Param::MappedDataArrayMode mode, uint64_t num_so,
                       bool export_visualization = true, bool use_pvsm = true) {
  auto set_param = [&](Param* param) {
    param->export_visualization_ = export_visualization;
    param->insitu_visualization_ = !export_visualization;
    param->visualization_export_generate_pvsm_ = use_pvsm;
    if (!export_visualization) {
      param->pv_insitu_pipeline_ =
          GetPythonScriptPath("validate_sim_objects.py");
      auto sim_name = Simulation::GetActive()->GetUniqueName();
      param->pv_insitu_pipeline_arguments_ =
          Concat("--sim_name=", sim_name, " --num_elements=", num_so);
    }
    param->run_mechanical_interactions_ = false;
    param->visualize_sim_objects_.insert(
        {"NeuriteElement", {"uid_", "daughter_right_"}});
    param->mapped_data_array_mode_ = mode;
  };
  neuroscience::InitModule();
  auto sim_name = Concat("ExportSimObjectsTest_", num_so, "_", mode);
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
    ne->SetDaughterRight(SoPointer<NeuriteElement>(SoUid(i)));
    rm->push_back(ne);
  };

  for (uint64_t i = 0; i < num_so; ++i) {
    construct(i);
  }

  // Don't run a simulation step, because neurites are not properly set up.
  (*sim->GetScheduler()->visualize_op_)();

  sim_name = sim->GetUniqueName();

  // NB: for insitu visualization the validation step happened in call Simulate
  if (export_visualization) {
    // create pvsm file
    delete sim;
    Validate("validate_sim_objects.py", sim_name, num_so, use_pvsm);
    ASSERT_TRUE(fs::exists(Concat(output_dir, "/valid")));
  } else {
    delete sim;
    ASSERT_TRUE(fs::exists(Concat(output_dir, "/valid")));
    exit(0);
  }
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportSimObjects_ZeroCopy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  RunSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunSimObjectsTest(mode, 10 * max_threads + 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportSimObjects_Cache) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCache;
  RunSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunSimObjectsTest(mode, 10 * max_threads + 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportSimObjects_Copy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCopy;
  RunSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunSimObjectsTest(mode, 10 * max_threads + 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, ExportSimObjectsLoadWithoutPVSM) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  RunSimObjectsTest(mode, std::max(1, max_threads - 1), true, false);
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, InsituSimObjects_ZeroCopy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  LAUNCH_IN_NEW_PROCESS(
      RunSimObjectsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunSimObjectsTest(mode, 10 * max_threads + 1, false));
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, InsituSimObjects_Cache) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCache;
  LAUNCH_IN_NEW_PROCESS(
      RunSimObjectsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunSimObjectsTest(mode, 10 * max_threads + 1, false));
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, DefaultInsituPipeline) {
  auto set_param = [](Param* param) {
    param->insitu_visualization_ = true;
    param->visualize_sim_objects_.insert({"Cell", {}});
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* cell = new Cell(30);
  rm->push_back(cell);

  simulation.GetScheduler()->Simulate(1);
  // Test passes if there is no crash
}

// -----------------------------------------------------------------------------
TEST(ParaviewIntegrationTest, InsituSimObjects_Copy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCopy;
  LAUNCH_IN_NEW_PROCESS(
      RunSimObjectsTest(mode, std::max(1, max_threads - 1), false));
  LAUNCH_IN_NEW_PROCESS(RunSimObjectsTest(mode, 10 * max_threads + 1, false));
}
}  // namespace bdm

#endif  // USE_PARAVIEW
