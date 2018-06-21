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

#include "visualization/catalyst_adaptor.h"
#include <gtest/gtest.h>
#include "io_util.h"
#include "unit/test_util.h"
#include "unit/default_ctparam.h"
#include "simulation_implementation.h"

// TODO(lukas) move file to unit/visualization

namespace bdm {

/// Test fixture for catalyst adaptor test to eliminate side effects
class CatalystAdaptorTest : public ::testing::Test {
 protected:
   static constexpr char const* kSimulationName = "MySimulation";
  static constexpr char const* kSimulationInfoJson = "output/MySimulation/simulation_info.json";
  static constexpr char const* kParaviewState = "output/MySimulation/MySimulation.pvsm";
  Simulation<>* simulation_;

  virtual void SetUp() {
    Simulation<>::counter_ = 0;
    simulation_ = new Simulation<>(kSimulationName);
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }

  virtual void TearDown() {
    delete simulation_;
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }
};

/// Tests if simulation_info.json is generated correctly during initialization.
TEST_F(CatalystAdaptorTest, GenerateSimulationInfoJson) {
  auto* param = simulation_->GetParam();

  // set-up Param values
  param->export_visualization_ = true;
  param->visualize_sim_objects_.clear();
  param->visualize_sim_objects_["cell"] = {};
  param->visualize_sim_objects_["neurite"] = {};
  param->visualize_diffusion_.clear();
  param->visualize_diffusion_.push_back({"sodium", true, true});

  std::unordered_map<std::string, Shape> shapes;
  shapes["cell"] = kSphere;
  shapes["neurite"] = kCylinder;

  CatalystAdaptor<>::GenerateSimulationInfoJson(shapes);

  ASSERT_TRUE(FileExists(kSimulationInfoJson));

  // check file contents
  std::ifstream ifs(kSimulationInfoJson);
  std::stringstream buffer;
  buffer << ifs.rdbuf();

  const char* expected = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"output/MySimulation"
  },
  "sim_objects": [
    {
      "name":"neurite",
      "glyph":"BDMGlyph",
      "shape":"Cylinder",
      "x_scaling_attribute":"diameter_",
      "y_scaling_attribute":"actual_length_",
      "z_scaling_attribute":"diameter_",
      "Vectors":"spring_axis_",
      "MassLocation":"mass_location_"
    },
    {
      "name":"cell",
      "glyph":"Glyph",
      "shape":"Sphere",
      "scaling_attribute":"diameter_"
    }
  ],
  "extracellular_substances": [
    { "name":"sodium", "has_gradient":"true" }
  ]
}
)STR";

  EXPECT_EQ(expected, buffer.str());
}

/// Tests if the catalyst state is generated.
TEST_F(CatalystAdaptorTest, GenerateParaviewState) {

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

  CatalystAdaptor<>::GenerateParaviewState();

  ASSERT_TRUE(FileExists(kParaviewState));
}

}  // namespace bdm
