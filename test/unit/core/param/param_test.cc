// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include <gtest/gtest.h>
#include <json.hpp>

#include "core/multi_simulation/optimization_param.h"
#include "core/multi_simulation/optimization_param_type/particle_swarm_param.h"
#include "unit/core/param/param_test.h"
#include "unit/test_util/test_util.h"

using nlohmann::json;

namespace bdm {

const ParamGroupUid TestParamGroup::kUid =
    ParamGroupUidGenerator::Get()->NewUid();

// -----------------------------------------------------------------------------
TEST(ParamTest, ToJsonString) {
  Param::RegisterParamGroup(new TestParamGroup());
  Param param;
  auto j_param = json::parse(param.ToJsonString());

  EXPECT_NEAR(3.14, j_param["bdm::TestParamGroup"]["test_param1"].get<double>(),
              abs_error<double>::value);
  EXPECT_EQ(42u, j_param["bdm::TestParamGroup"]["test_param2"].get<uint64_t>());
  EXPECT_EQ(-1, j_param["bdm::TestParamGroup"]["test_param3"].get<int>());
  EXPECT_EQ("output", j_param["bdm::Param"]["output_dir"].get<std::string>());
}

// -----------------------------------------------------------------------------
TEST(ParamTest, RestoreFromJson) {
  Param::RegisterParamGroup(new TestParamGroup());
  Param param;

  std::string patch1 = R"EOF(
{
  "bdm::Param": {
    "visualize_agents": {
      "Cell": ["type"]
    }
  },
  "bdm::TestParamGroup": {
    "test_param1": 6.28,
    "test_param3": -10
  }
}
)EOF";

  std::string patch2 = R"EOF(
{
  "bdm::Param": {
    "simulation_time_step" : 1.0,
    "visualize_agents": {
      "Cell": ["type", "some-dm"]
    }
  },
  "bdm::TestParamGroup": {
    "test_param2": 123
  }
}
)EOF";

  param.MergeJsonPatch(patch1);

  EXPECT_EQ(1u, param.visualize_agents.size());
  auto vis_cell = param.visualize_agents["Cell"];
  EXPECT_EQ(1u, vis_cell.size());
  EXPECT_TRUE(vis_cell.find("type") != vis_cell.end());

  auto* test_param = param.Get<TestParamGroup>();
  EXPECT_NEAR(6.28, test_param->test_param1, abs_error<double>::value);
  EXPECT_EQ(42u, test_param->test_param2);
  EXPECT_EQ(-10, test_param->test_param3);

  param.MergeJsonPatch(patch2);

  EXPECT_EQ(1u, param.visualize_agents.size());
  vis_cell = param.visualize_agents["Cell"];
  EXPECT_EQ(2u, vis_cell.size());
  EXPECT_TRUE(vis_cell.find("type") != vis_cell.end());
  EXPECT_TRUE(vis_cell.find("some-dm") != vis_cell.end());

  test_param = param.Get<TestParamGroup>();
  EXPECT_NEAR(6.28, test_param->test_param1, abs_error<double>::value);
  EXPECT_EQ(123u, test_param->test_param2);
  EXPECT_EQ(-10, test_param->test_param3);
}

TEST(ParamTest, OptimizationParam) {
  Param param;
  auto* opt_param = param.Get<OptimizationParam>();

  EXPECT_TRUE(opt_param != nullptr);
  EXPECT_EQ("", opt_param->algorithm);
  EXPECT_EQ(0u, opt_param->params.size());
  EXPECT_EQ(1u, opt_param->repetition);
  EXPECT_EQ(100u, opt_param->max_iterations);

  std::string patch = R"EOF(
{
  "bdm::OptimizationParam": {
    "algorithm" : "ParticleSwarm",
    "repetition" : 10,
    "max_iterations" : 1000,
    "params" : [
      {
        "_typename": "bdm::ParticleSwarmParam",
        "param_name" : "bdm::SimParam::infection_probablity",
        "lower_bound" : 0.001,
        "upper_bound" : 1,
        "initial_value" : 0.005
      },
      {
        "_typename": "bdm::ParticleSwarmParam",
        "param_name" : "bdm::SimParam::infection_radius",
        "lower_bound" : 5,
        "upper_bound" : 50,
        "initial_value" : 5
      },
      {
        "_typename": "bdm::ParticleSwarmParam",
        "param_name" : "bdm::SimParam::agent_speed",
        "lower_bound" : 2,
        "upper_bound" : 50,
        "initial_value" : 2
      }
    ]
  }
}
)EOF";

  param.MergeJsonPatch(patch);
  opt_param = param.Get<OptimizationParam>();
  EXPECT_EQ("ParticleSwarm", opt_param->algorithm);
  EXPECT_EQ(3u, opt_param->params.size());
  auto* swarm_param = static_cast<ParticleSwarmParam*>(opt_param->params[0]);
  EXPECT_EQ("bdm::SimParam::infection_probablity",
            swarm_param->param_name);
  EXPECT_EQ(0.001, swarm_param->lower_bound);
  EXPECT_EQ(1, swarm_param->upper_bound);
  EXPECT_EQ(0.005, swarm_param->initial_value);
  EXPECT_EQ(10u, opt_param->repetition);
  EXPECT_EQ(1000u, opt_param->max_iterations);
}

}  // namespace bdm
