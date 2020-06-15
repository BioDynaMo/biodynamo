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

#include <experimental/filesystem>
#include <gtest/gtest.h>

#include "biodynamo.h"
#include "neuroscience/neuroscience.h"
#include "core/visualization/visualization_adaptor.h"

namespace fs = std::experimental::filesystem;

namespace bdm {

// -----------------------------------------------------------------------------
void Validate(const std::string& python_script, 
              const std::string& sim_name, 
              uint64_t num_elements) {
  std::stringstream cmd;
  std::string bdm_sys_env = std::getenv("BDMSYS");
   
  cmd << bdm_sys_env << "/third_party/paraview/bin/pvpython "
             << bdm_sys_env
             << "/share/test/core/visualization/paraview/" << python_script
             << " --sim_name=" << sim_name 
             << " --num_elements=" << num_elements;
  int ret_code = system(cmd.str().c_str());
  EXPECT_EQ(0, ret_code);
}

// -----------------------------------------------------------------------------
void RunExportDiffusionGridTest(uint64_t max_bound, uint64_t resolution) {
  auto set_param = [&](Param* param) {
    param->min_bound_ = 0; 
    param->max_bound_ = max_bound;
    param->export_visualization_ = true;
    param->visualize_diffusion_.push_back({"Substance", true});
  };
  auto sim_name = Concat("ExportDiffusionGridTest_", max_bound, "_", resolution);
  auto* sim = new Simulation(sim_name, set_param);
  auto output_dir = sim->GetOutputDir();
  fs::remove_all(output_dir); 
  fs::create_directory(output_dir); 
  
  ModelInitializer::DefineSubstance(0, "Substance", 0.0001, 0.001, resolution);
  // create a sequence 1, 2, 3...
  // since initialization is multithreaded returning in increasing counter
  // does not work. -> calculate and return box id
  ModelInitializer::InitializeSubstance(0, "Substance", [&](double x, double y, double z) {
    auto* dg = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(0);
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
  auto *cell = new Cell();
  cell->SetDiameter(10);
  cell->SetPosition({5, 5, 5});
  sim->GetResourceManager()->push_back(cell);
  auto *cell1 = new Cell();
  cell1->SetDiameter(10);
  double pos = static_cast<double>(max_bound) - 5;
  cell1->SetPosition({pos, pos, pos});
  sim->GetResourceManager()->push_back(cell1);

  sim->GetScheduler()->Simulate(1);
 
  auto* dg = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(0);
  auto num_boxes = dg->GetNumBoxes();
  sim_name = sim->GetUniqueName();
  
  // create pvsm file
  delete sim;

  Validate("validate_diffusion_grid.py", sim_name, num_boxes); 
}

// -----------------------------------------------------------------------------
TEST(ParaviewFullCycleTest, ExportDiffusionGrid_SlicesLtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  RunExportDiffusionGridTest(max_threads - 1, max_threads - 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewFullCycleTest, ExportDiffusionGrid_SlicesGtNumThreads) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  RunExportDiffusionGridTest(3 * max_threads + 1, max_threads);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void RunExportSimObjectsTest(Param::MappedDataArrayMode mode, 
                             uint64_t num_so) {
  auto set_param = [&](Param* param) {
    param->run_mechanical_interactions_ = false;
    param->export_visualization_ = true;
    param->visualize_sim_objects_.insert({"NeuriteElement", {"uid_", "daughter_right_"}});
    param->mapped_data_array_mode_= mode;
  };
  experimental::neuroscience::InitModule();
  auto sim_name = Concat("ExportSimObjectsTest_", num_so, "_", mode);
  auto* sim = new Simulation(sim_name, set_param);

  auto output_dir = sim->GetOutputDir();
  fs::remove_all(output_dir); 
  fs::create_directory(output_dir); 
  
  using NeuriteElement = experimental::neuroscience::NeuriteElement; 

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
  auto vis = VisualizationAdaptor::Create("paraview");
  vis->Visualize();

  sim_name = sim->GetUniqueName();
  
  // create pvsm file
  delete vis;

  Validate("validate_sim_objects.py", sim_name, num_so); 
  
  // deleting sim would overwrite the pvsm and json file
  fs::path pvsm = Concat("output/", sim_name, "/", sim_name, ".pvsm"); 
  fs::path json = Concat("output/", sim_name, "/simulation_info.json");
  fs::path pvsm_tmp = Concat(pvsm.string(), ".tmp"); 
  fs::path json_tmp = Concat(json.string(), ".tmp"); 
  fs::rename(pvsm, pvsm_tmp);
  fs::rename(json, json_tmp);
  delete sim;
  fs::remove(pvsm);
  fs::remove(json);
  fs::rename(pvsm_tmp, pvsm);
  fs::rename(json_tmp, json);
}

// -----------------------------------------------------------------------------
TEST(ParaviewFullCycleTest, ExportSimObjects_ZeroCopy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kZeroCopy;
  RunExportSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunExportSimObjectsTest(mode, 10 * max_threads + 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewFullCycleTest, ExportSimObjects_Cache) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCache;
  RunExportSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunExportSimObjectsTest(mode, 10 * max_threads + 1);
}

// -----------------------------------------------------------------------------
TEST(ParaviewFullCycleTest, ExportSimObjects_Copy) {
  auto max_threads = ThreadInfo::GetInstance()->GetMaxThreads();
  auto mode = Param::MappedDataArrayMode::kCopy;
  RunExportSimObjectsTest(mode, std::max(1, max_threads - 1));
  RunExportSimObjectsTest(mode, 10 * max_threads + 1);
}

}  // namespace bdm

#endif  // USE_PARAVIEW
