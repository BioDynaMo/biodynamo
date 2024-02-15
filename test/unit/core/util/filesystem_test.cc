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
#include <filesystem>
#include <fstream>

namespace bdm {

TEST(FileSystemTest, RemoveDirectoryContents) {
  std::filesystem::remove_all("file-system-test");
  std::filesystem::create_directory("file-system-test");

  // ignore empty directory check
  EXPECT_EQ(0u, RemoveDirectoryContents("file-system-test"));
  EXPECT_TRUE(std::filesystem::exists("file-system-test"));
  // ignore files check
  std::ofstream ofs("file-system-test/file");
  ofs << "This is a test" << std::endl;
  EXPECT_EQ(0u, RemoveDirectoryContents("file-system-test/file"));
  EXPECT_TRUE(std::filesystem::exists("file-system-test/file"));
  // check if directory is emptied
  EXPECT_EQ(1u, RemoveDirectoryContents("file-system-test"));
  EXPECT_FALSE(std::filesystem::exists("file-system-test/file"));

  // clean-up
  std::filesystem::remove_all("file-system-test");
}

}  // namespace bdm
