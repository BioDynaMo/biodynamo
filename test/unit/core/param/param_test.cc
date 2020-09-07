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

#include "unit/core/param/param_test.h"
#include <gtest/gtest.h>
#include <json.hpp>
#include "unit/test_util/test_util.h"

using nlohmann::json;

namespace bdm {

const ModuleParamUid TestModuleParam::kUid =
    ModuleParamUidGenerator::Get()->NewUid();

// -----------------------------------------------------------------------------
TEST(ParamTest, ToJsonString) {
  Param::RegisterModuleParam(new TestModuleParam());
  Param param;
  auto j_param = json::parse(param.ToJsonString());

  EXPECT_NEAR(3.14,
              j_param["bdm::TestModuleParam"]["test_param1"].get<double>(),
              abs_error<double>::value);
  EXPECT_EQ(42u,
            j_param["bdm::TestModuleParam"]["test_param2"].get<uint64_t>());
  EXPECT_EQ(-1, j_param["bdm::TestModuleParam"]["test_param3"].get<int>());
  EXPECT_EQ("output", j_param["bdm::Param"]["output_dir_"].get<std::string>());
}

// -----------------------------------------------------------------------------
TEST(ParamTest, RestoreFromJson) {
  Param::RegisterModuleParam(new TestModuleParam());
  Param param;

  std::string patch1 = R"EOF(
{
  "bdm::Param": {
    "visualize_sim_objects_": {
      "Cell": ["type"]
    }
  },
  "bdm::TestModuleParam": {
    "test_param1": 6.28,
    "test_param3": -10
  }
}
)EOF";

  std::string patch2 = R"EOF(
{
  "bdm::Param": {
    "simulation_time_step_" : 1.0,
    "visualize_sim_objects_": {
      "Cell": ["type", "some-dm"]
    }
  },
  "bdm::TestModuleParam": {
    "test_param2": 123
  }
}
)EOF";

  param.MergeJsonPatch(patch1);

  EXPECT_EQ(1u, param.visualize_sim_objects_.size());
  auto vis_cell = param.visualize_sim_objects_["Cell"];
  EXPECT_EQ(1u, vis_cell.size());
  EXPECT_TRUE(vis_cell.find("type") != vis_cell.end());

  auto* test_param = param.GetModuleParam<TestModuleParam>();
  EXPECT_NEAR(6.28, test_param->test_param1, abs_error<double>::value);
  EXPECT_EQ(42u, test_param->test_param2);
  EXPECT_EQ(-10, test_param->test_param3);

  param.MergeJsonPatch(patch2);

  EXPECT_EQ(1u, param.visualize_sim_objects_.size());
  vis_cell = param.visualize_sim_objects_["Cell"];
  EXPECT_EQ(2u, vis_cell.size());
  EXPECT_TRUE(vis_cell.find("type") != vis_cell.end());
  EXPECT_TRUE(vis_cell.find("some-dm") != vis_cell.end());

  test_param = param.GetModuleParam<TestModuleParam>();
  EXPECT_NEAR(6.28, test_param->test_param1, abs_error<double>::value);
  EXPECT_EQ(123u, test_param->test_param2);
  EXPECT_EQ(-10, test_param->test_param3);
}

}  // namespace bdm
