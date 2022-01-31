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

#include "core/util/proc.h"
#include <unistd.h>
#include <experimental/filesystem>
#include "core/util/log.h"

namespace fs = std::experimental::filesystem;

#ifdef LINUX

namespace bdm {

std::string GetExecutablePath() {
  char buffer[1024];
  int path_size = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (path_size <= 0) {
    Log::Fatal("GetExecutablePath",
               "readlink(\"/proc/self/exe\", ...)  failed");
  }
  // return value of readlink is not null terminated
  buffer[path_size] = '\0';
  return std::string(buffer);
}

}  // namespace bdm

#else  // APPLE

#include <libproc.h>
#include <cstdlib>

namespace bdm {

std::string GetExecutablePath() {
  pid_t pid = getpid();
  char buffer[1024];
  int path_size = proc_pidpath(pid, buffer, sizeof(buffer));
  if (path_size <= 0) {
    Log::Fatal("GetExecutablePath",
               "readlink(\"/proc/self/exe\", ...)  failed");
  }
  return std::string(buffer);
}

}  // namespace bdm

#endif  // LINUX

namespace bdm {

std::string GetExecutableDirectory() {
  fs::path bin_path = GetExecutablePath();
  return bin_path.remove_filename().string();
}

std::string GetExecutableName() {
  fs::path bin_path = GetExecutablePath();
  return bin_path.filename().string();
}

}  // namespace bdm
