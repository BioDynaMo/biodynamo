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

#include "core/util/filesystem.h"
#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace bdm {

uint64_t RemoveDirectoryContents(const std::string& directory) {
  fs::path dir = directory;
  if (!fs::is_directory(dir) || fs::is_empty(dir)) {
    return 0;
  }
  auto files_removed = fs::remove_all(directory);
  fs::create_directory(directory);
  // subtract 1 because we don't count the removal of `directory`
  // itself.
  return files_removed - 1;
}

}  // namespace bdm
