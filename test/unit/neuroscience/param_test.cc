// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/param/param.h"
#include "core/simulation.h"
#include "neuroscience/module.h"
#include "neuroscience/param.h"

#include "unit/test_util/test_util.h"

namespace bdm {
namespace neuroscience {

TEST(neuroscience, Param) {
  constexpr const char* kConfigFileName = "bdm.toml";
  constexpr const char* kConfigContent =
      "[neuroscience]\n"
      "neurite_default_actual_length = 2.0\n"
      "neurite_default_density = 3.0\n"
      "neurite_default_diameter = 4.0\n"
      "neurite_default_spring_constant = 5.0\n"
      "neurite_default_adherence = 6.0\n"
      "neurite_default_tension = 7.0\n"
      "neurite_min_length = 8.0\n"
      "neurite_max_length = 9.0\n"
      "neurite_minimial_bifurcation_length = 10.0\n";

  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  auto* param = simulation.GetParam()->Get<Param>();

  ASSERT_TRUE(param != nullptr);
  EXPECT_EQ(2.0, param->neurite_default_actual_length);
  EXPECT_EQ(3.0, param->neurite_default_density);
  EXPECT_EQ(4.0, param->neurite_default_diameter);
  EXPECT_EQ(5.0, param->neurite_default_spring_constant);
  EXPECT_EQ(6.0, param->neurite_default_adherence);
  EXPECT_EQ(7.0, param->neurite_default_tension);
  EXPECT_EQ(8.0, param->neurite_min_length);
  EXPECT_EQ(9.0, param->neurite_max_length);
  EXPECT_EQ(10.0, param->neurite_minimial_bifurcation_length);

  remove(kConfigFileName);
}

}  // namespace neuroscience
}  // namespace bdm
