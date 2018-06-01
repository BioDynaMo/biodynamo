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

#include "biodynamo.h"
#include <cstdlib>
#include <string>
#include "command_line_options.h"
#include "cpptoml/cpptoml.h"
#include "version.h"

namespace bdm {

/// Return only the executable name given the path
/// @param path path and filename of the executable
/// e.g. `executable`, `./executable`, './build/executable'
/// @return `executabl`
std::string ExtractExecutableName(const char* path) {
  std::string s(path);
  auto pos = s.find_last_of("/");
  if (pos == std::string::npos) {
    return s;
  } else {
    return s.substr(pos + 1, s.length() - 1);
  }
}

void InitializeBiodynamo(int argc, const char** argv) {
  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initializing BiodynaMo ", Version::String());

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDM_CMAKE_DIR") == nullptr) {
    Log::Fatal("InitializeBiodynamo",
               "The BioDynaMo environment is not set up correctly. Please call "
               "$use_biodynamo "
               "and retry this command.");
  }

  Param::executable_name_ = ExtractExecutableName(argv[0]);
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  constexpr auto kConfigFile = "bdm.toml";
  constexpr auto kConfigFileParentDir = "../bdm.toml";
  if (FileExists(kConfigFile)) {
    auto config = cpptoml::parse_file(kConfigFile);
    Param::AssignFromConfig(config);
  } else if (FileExists(kConfigFileParentDir)) {
    auto config = cpptoml::parse_file(kConfigFileParentDir);
    Param::AssignFromConfig(config);
  } else {
    Log::Warning("InitializeBiodynamo",
                 "Config file %s not found in `.` or `../` directory.",
                 kConfigFile);
  }
  if (options.backup_file_ != "") {
    Param::backup_file_ = options.backup_file_;
    Param::restore_file_ = options.restore_file_;
  }
}

void InitializeBiodynamo(const std::string& executable_name) {
  const char* argv[1] = {executable_name.c_str()};
  InitializeBiodynamo(1, argv);
}

}  // namespace bdm
