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

/// Class to contain and parse command line options
class CommandLineOptions {
 public:
  CommandLineOptions(int argc, const char** argv);

  ~CommandLineOptions();

  /// Add an extra command line option
  cxxopts::OptionAdder AddOption(std::string group = "Simulation");

  /// Add an extra command line option
  template <typename T>
  void AddOption(std::string opt, std::string def, std::string description = "",
                 std::string group = "Simulation") {
    AddOption(group)(opt, description, cxxopts::value<T>()->default_value(def));
  }

  /// Return the simulation name that was parsed from argv[0]
  std::string GetSimulationName();

  template <typename T>
  T Get(std::string val) {
    if (parser_ == nullptr) {
      this->Parse();
    }
    return parser_->Get<T>(val);
  }

 private:
  void AddCoreOptions();

  /// Parse the options with the given command line arguments
  void Parse();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  void ExtractSimulationName(const char* path);

  /// Takes care of core options
  void HandleCoreOptions();

  int argc_;
  const char** argv_;
  // The name of the simulation
  std::string sim_name_;

  // Flag to determine if new options were added
  bool first_parse_ = true;
  cxxopts::Options options_;
  cxxopts::ParseResult* parser_ = nullptr;
};

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
