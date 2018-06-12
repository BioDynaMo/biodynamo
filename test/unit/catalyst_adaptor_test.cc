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
#include "bdm_imp.h"

// TODO(lukas) move file to unit/visualization

namespace bdm {

/// Test fixture for catalyst adaptor test to eliminate side effects
class CatalystAdaptorTest : public ::testing::Test {
 protected:
  static constexpr char const* kSimulationInfoJson = "simulation_info.json";
  static constexpr char const* kSimulationName = "MySimulation";
  static constexpr char const* kParaviewState = "MySimulation.pvsm";

  virtual void TearDown() {
    Param::Reset();
    remove(kSimulationInfoJson);
    remove(kParaviewState);
  }
};

/// Tests if simulation_info.json is generated correctly during initialization.
TEST_F(CatalystAdaptorTest, GenerateSimulationInfoJson) {
  // remove files to avoid false positive test results
  remove(kSimulationInfoJson);
  remove(kParaviewState);

  // set-up Param values
  Param::export_visualization_ = true;
  Param::visualize_sim_objects_.clear();
  Param::visualize_sim_objects_["cell"] = {};
  Param::visualize_sim_objects_["neurite"] = {};
  Param::visualize_diffusion_.clear();
  Param::visualize_diffusion_.push_back({"sodium", true, true});
  Param::executable_name_ = kSimulationName;

  std::unordered_map<std::string, Shape> shapes;
  shapes["cell"] = kSphere;
  shapes["neurite"] = kCylinder;

  CatalystAdaptor<>::GenerateSimulationInfoJson(shapes);

  Param::Reset();

  ASSERT_TRUE(FileExists(kSimulationInfoJson));

  // check file contents
  std::ifstream ifs(kSimulationInfoJson);
  std::stringstream buffer;
  buffer << ifs.rdbuf();

  const char* expected = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"."
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
  remove(kSimulationInfoJson);
  const char* empty_json = R"STR({
  "simulation": {
    "name":"MySimulation",
    "result_dir":"."
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
