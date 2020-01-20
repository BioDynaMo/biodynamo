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

#include "core/param/command_line_options.h"
#include "core/simulation.h"

#include <string>

#include "gtest/gtest.h"

namespace bdm {

TEST(CommandLineOptionsTest, CoreOptions) {
  const char* argv[1] = {"./binary_name"};
  CommandLineOptions clo(1, argv);
  Simulation simulation(&clo);

  EXPECT_EQ("binary_name", clo.GetSimulationName());
  EXPECT_EQ("", clo.Get<std::string>("config"));
  EXPECT_EQ("", clo.Get<std::string>("backup"));
  EXPECT_EQ("", clo.Get<std::string>("restore"));
}

TEST(CommandLineOptionsTest, MultipleOptions) {
  const char* argv[2] = {"./binary_name", "--output=mydir"};
  CommandLineOptions clo(2, argv);

  clo.AddOption<std::string>("output", "some_dir");
  clo.AddOption<std::string>("foo", "bar");

  Simulation simulation(&clo);

  EXPECT_EQ("mydir", clo.Get<std::string>("output"));
  EXPECT_EQ("bar", clo.Get<std::string>("foo"));
}

TEST(CommandLineOptionsTest, NoSimulationObject) {
  const char* argv[2] = {"./binary_name", "--output=mydir"};
  CommandLineOptions clo(2, argv);

  // AddOption internally will trigger Parse() when we Get
  clo.AddOption<std::string>("output", "some_dir");
  EXPECT_EQ("mydir", clo.Get<std::string>("output"));
}

}  // namespace bdm
