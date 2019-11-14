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

#ifndef CORE_PARAM_COMMAND_LINE_OPTIONS_H_
#define CORE_PARAM_COMMAND_LINE_OPTIONS_H_

#include "cxxopts.h"

#include "TError.h"

#include "core/simulation.h"
#include "version.h"

#include <algorithm>
#include <iostream>
#include <string>

namespace bdm {

using cxxopts::value;
using std::string;

/// Class to contain and parse command line options
class CommandLineOptions {
 public:
  CommandLineOptions(int argc, const char** argv);

  /// Add an extra command line option
  cxxopts::OptionAdder AddOption(string group = "");

  /// Add an extra command line option
  template <typename T>
  void AddOption(string opt, string description, string def,
                 string group = "Simulation") {
    AddOption(group)(opt, description, cxxopts::value<T>()->default_value(def));
  }

  /// Return the simulation name that was parsed from argv[0]
  std::string GetSimulationName();

  /// Parse the given command line arguments
  cxxopts::ParseResult Parse();

 private:
  void AddCoreOptions();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  void ExtractSimulationName(const char* path);

  void HandleCoreOptions(cxxopts::ParseResult& ret);

  int argc_;
  const char** argv_;
  string sim_name_;
  cxxopts::Options options_;
};

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
