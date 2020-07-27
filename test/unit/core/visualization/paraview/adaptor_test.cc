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
#include "core/util/io.h"
#include "core/visualization/paraview/adaptor.h"
#include "core/visualization/paraview/helper.h"
#include "unit/core/visualization/paraview/adaptor_test.h"
#include "unit/test_util/test_util.h"

namespace fs = std::experimental::filesystem;

namespace bdm {

using MyCell = paraview_adaptor_test_internal::MyCell;
using MyNeuron = paraview_adaptor_test_internal::MyNeuron;

/// Test fixture for catalyst adaptor test to eliminate side effects
class ParaviewAdaptorTest : public ::testing::Test {
 protected:
  static constexpr char const* kSimulationName = "MySimulation";
  static constexpr char const* kSimulationInfoJson =
      "output/MySimulation/simulation_info.json";
  static constexpr char const* kParaviewState =
      "output/MySimulation/MySimulation.pvsm";

  virtual void SetUp() {
    Simulation::counter_ = 0;
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }

  virtual void TearDown() {
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }
};

/// Tests if simulation_info.json is generated correctly during initialization.
TEST_F(ParaviewAdaptorTest, GenerateSimulationInfoJson) {
  auto set_param = [](auto* param) {
    // set-up Param values
    param->export_visualization_ = true;
    param->visualize_sim_objects_.clear();
    param->visualize_sim_objects_["Cell"] = {};
    param->visualize_sim_objects_["NeuriteElement"] = {};
    param->visualize_diffusion_.clear();
    param->visualize_diffusion_.push_back({"sodium", true, true});
  };

  Simulation simulation(kSimulationName, set_param);

  // create internal objects
  vtkCPDataDescription* data_description = vtkCPDataDescription::New();
  std::unordered_map<std::string, VtkSimObjects*> vtk_sim_objects;
  vtk_sim_objects["Cell"] = new VtkSimObjects("Cell", data_description);
  vtk_sim_objects["Cell"]->shape_ = kSphere;
  vtk_sim_objects["NeuriteElement"] = new VtkSimObjects("NeuriteElement", data_description);
  vtk_sim_objects["NeuriteElement"]->shape_ = kCylinder;

  std::unordered_map<std::string, VtkDiffusionGrid*> vtk_dgrids;
  vtk_dgrids["sodium"] = new VtkDiffusionGrid("sodium", data_description);
  vtk_dgrids["sodium"]->used_ = true;

  auto json = GenerateSimulationInfoJson(vtk_sim_objects, vtk_dgrids);

  // free memory
  for (auto& el : vtk_sim_objects) {
    delete el.second;
  }
  for (auto& el : vtk_dgrids) {
    delete el.second;
  }
  data_description->Delete();

  const char* expected = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"output/MySimulation"
  },
  "sim_objects": [
    {
      "name":"NeuriteElement",
      "glyph":"BDMGlyph",
      "shape":"Cylinder",
      "x_scaling_attribute":"diameter_",
      "y_scaling_attribute":"actual_length_",
      "z_scaling_attribute":"diameter_",
      "Vectors":"spring_axis_",
      "MassLocation":"mass_location_"
    },
    {
      "name":"Cell",
      "glyph":"Glyph",
      "shape":"Sphere",
      "scaling_attribute":"diameter_"
    }
  ],
  "extracellular_substances": [
    { "name":"sodium", "has_gradient":"true" }
  ],
  "insitu_script_arguments": ""
}
)STR";

  EXPECT_EQ(expected, json);
}

TEST_F(ParaviewAdaptorTest, OmitPvsmAndJsonGeneration) {
  auto set_param = [](Param* param) {
    param->export_visualization_ = true;
    param->visualization_export_generate_pvsm_ = false;
  };

  auto* sim = new Simulation(TEST_NAME, set_param);

  auto json_filename =
      Concat("output/", sim->GetOutputDir(), "/simulation_info.json");
  auto pvsm_filename = Concat("output/", sim->GetOutputDir(), "/",
                              sim->GetUniqueName(), ".pvsm");

  delete sim;

  EXPECT_FALSE(FileExists(json_filename));
  EXPECT_FALSE(FileExists(pvsm_filename));
}

/// Tests if the catalyst state is generated.
TEST_F(ParaviewAdaptorTest, GenerateParaviewState) {
  Simulation simulation("MySimulation");
  // before we can call finalize we need to modify the json object
  // we need to remove entries for sim_objects and extracellular_substances
  // because there are no corresponding data files available.
  // Therefore the script would fail.
  const char* empty_json = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"output/MySimulation"
  },
  "sim_objects": [],
  "extracellular_substances": []
}
)STR";

  std::ofstream ofs;
  ofs.open(kSimulationInfoJson);
  ofs << empty_json;
  ofs.close();

  ParaviewAdaptor::GenerateParaviewState();

  ASSERT_TRUE(FileExists(kParaviewState));
}

/// Test if the objects that we want to output for visualization are indeed
/// the only ones (no more, no less).
TEST_F(ParaviewAdaptorTest, CheckVisualizationSelection) {
  auto set_param = [](auto* param) {
    param->export_visualization_ = true;

    // We selection Substance_1 for export
    Param::VisualizeDiffusion vd;
    vd.name_ = "Substance_1";
    param->visualize_diffusion_.push_back(vd);

    // We select MyCell for export
    param->visualize_sim_objects_["MyCell"] = {};
    param->visualize_sim_objects_["Cell"] = {};
  };
  auto status = std::system(Concat("rm -f output/", TEST_NAME, "/*").c_str());
  if (status != 0) {
    Log::Warning(TEST_NAME,
                 "Error during removal of Paraview files -- status code: ",
                 status);
  }

  Simulation sim(TEST_NAME, set_param);
  auto output_dir = sim.GetOutputDir();
  fs::remove_all(output_dir); 
  fs::create_directory(output_dir); 

  auto* rm = sim.GetResourceManager();

  enum Substances { kSubstance0, kSubstance1, kSubstance2 };

  // Create two types of cells
  rm->push_back(new MyCell());
  rm->push_back(new MyCell());
  rm->push_back(new MyNeuron());

  // Define the substances
  ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.05, 0, 5);
  ModelInitializer::DefineSubstance(kSubstance2, "Substance_2", 0.05, 0, 5);
  ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.05, 0, 5);

  int l = -100;
  int r = 100;
  rm->GetDiffusionGrid(kSubstance0)->Initialize({l, r, l, r, l, r});
  rm->GetDiffusionGrid(kSubstance1)->Initialize({l, r, l, r, l, r});
  rm->GetDiffusionGrid(kSubstance2)->Initialize({l, r, l, r, l, r});

  // Write diffusion visualization to file
  sim.Simulate(3);

  std::string pvsm = TEST_NAME + std::string(".pvsm");

  // Check if all the output files of the selected items were generated
  // Don't check for vtu or vti files. This depends on the number
  // of threads and data.
  // Other tests cover this.
  std::set<std::string> required_files = {"Cell-0.pvtu",
                                          "Cell-1.pvtu",
                                          "Cell-2.pvtu",
                                          "MyCell-0.pvtu",
                                          "MyCell-1.pvtu",
                                          "MyCell-2.pvtu",
                                          "Substance_1-0.pvti",
                                          "Substance_1-1.pvti",
                                          "Substance_1-2.pvti"};

  for (auto& file : required_files) {
    EXPECT_TRUE(fs::exists(Concat(sim.GetOutputDir(), "/", file)));
  }
}

}  // namespace bdm

#endif  // USE_PARAVIEW
