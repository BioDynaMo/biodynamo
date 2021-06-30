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

#ifndef CORE_UTIL_PROC_H_
#define CORE_UTIL_PROC_H_

#include <string>

namespace bdm {

/// This function returns the path of the currently running executable
std::string GetExecutablePath();

std::string GetExecutableDirectory();

std::string GetExecutableName();

}  // namespace bdm

#endif  // CORE_UTIL_PROC_H_
