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

#ifndef CORE_UTIL_FILESYSTEM_H_
#define CORE_UTIL_FILESYSTEM_H_

#include <cstdint>
#include <string>

namespace bdm {

/// Removes all contents inside the directory and returns
/// the number of deleted files and directories that were
/// deleted.
uint64_t RemoveDirectoryContents(const std::string& directory);

}  // namespace bdm

#endif  // CORE_UTIL_FILESYSTEM_H_
