// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/util/filesystem.h"
#include <gtest/gtest.h>
#include <experimental/filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;

namespace bdm {

TEST(FileSystemTest, RemoveDirectoryContents) {
  fs::remove_all("file-system-test");
  fs::create_directory("file-system-test");

  // ignore empty directory check
  EXPECT_EQ(0u, RemoveDirectoryContents("file-system-test"));
  EXPECT_TRUE(fs::exists("file-system-test"));
  // ignore files check
  std::ofstream ofs("file-system-test/file");
  ofs << "This is a test" << std::endl;
  EXPECT_EQ(0u, RemoveDirectoryContents("file-system-test/file"));
  EXPECT_TRUE(fs::exists("file-system-test/file"));
  // check if directory is emptied
  EXPECT_EQ(1u, RemoveDirectoryContents("file-system-test"));
  EXPECT_FALSE(fs::exists("file-system-test/file"));

  // clean-up
  fs::remove_all("file-system-test");
}

}  // namespace bdm
