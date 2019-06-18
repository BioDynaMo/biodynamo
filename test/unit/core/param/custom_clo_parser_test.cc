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

#include "core/param/custom_clo_parser.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(CustomCLOParserTest, All) {
  const char* argv[4] = {"./binary_name", "--iterations=123",
                         "--default-diameter=3.14", "--sim-name=MySimulation"};
  CustomCLOParser parser(4, argv);
  EXPECT_EQ(123, parser.GetValue<int>("iterations"));
  EXPECT_NEAR(3.14, parser.GetValue<double>("default-diameter"), 1e-5);
  EXPECT_EQ("MySimulation", parser.GetValue<std::string>("sim-name"));

  EXPECT_EQ(321, parser.GetValue<int>("missing-param", true, 321));

  try {
    parser.GetValue<int>("missing-param", false);
    FAIL() << "Exception expected" << std::endl;
  } catch (const std::logic_error& e) {
  }
}

}  // namespace bdm
