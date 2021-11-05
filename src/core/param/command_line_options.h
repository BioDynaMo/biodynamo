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

#ifndef CORE_PARAM_COMMAND_LINE_OPTIONS_H_
#define CORE_PARAM_COMMAND_LINE_OPTIONS_H_

/// Don't split at ',' if options are repeated
/// This would split { "bdm::Param": "option1": 123, "option2": 123}}
/// into two strings: "{ "bdm::Param": "option1": 123" and ""option2": 123}}"
#define CXXOPTS_VECTOR_DELIMITER '\n'
#include <TError.h>
#include <cxxopts.h>

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

#include "bdm_version.h"
#include "core/simulation.h"

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
  void AddOption(const std::string& opt, std::string def,
                 const std::string& description = "",
                 std::string group = "Simulation") {
    AddOption(std::move(group))(opt, description,
                                cxxopts::value<T>()->default_value(def));
  }

  /// Return the simulation name that was parsed from argv[0]
  std::string GetSimulationName();

  template <typename T>
  T Get(const std::string& val) {
    if (parser_ == nullptr) {
      this->Parse();
    }
    return (*parser_)[val].as<T>();
  }

 private:
  friend std::ostream& operator<<(std::ostream& os,
                                  const CommandLineOptions& clo);

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
