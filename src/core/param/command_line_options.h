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
  CommandLineOptions(int argc, const char** argv)
      : argc_(argc),
        argv_(argv),
        options_(argv[0], " -- BioDynaMo command line options\n") {
    AddCoreOptions();
  }

  /// Add an extra command line option
  cxxopts::OptionAdder AddOption(string group = "") {
    return cxxopts::OptionAdder(options_, std::move(group));
  }

  template <typename T>
  void AddOption(std::string opt, std::string description, std::string def,
                 std::string group = "Simulation") {
    AddOption(group)(opt, description, cxxopts::value<T>()->default_value(def));
  }

  std::string GetSimulationName() { return sim_name_; }

  /// Parse the given command line arguments
  cxxopts::ParseResult Parse() {
    // Make a non-const deep copy of argv
    char** argv_copy = (char**)malloc((argc_ + 1) * sizeof(char*));
    for (int i = 0; i < argc_; ++i) {
      size_t length = strlen(argv_[i]);
      argv_copy[i] = (char*)malloc(length);
      memcpy(argv_copy[i], argv_[i], length);
    }
    argv_copy[argc_] = NULL;

    // Perform parsing (consumes argv_copy)
    auto ret = options_.parse(argc_, argv_copy);

    // Perform operations on Core command line options
    HandleCoreOptions(ret);

    // free memory
    for (int i = 0; i < argc_; ++i) {
      free(argv_copy[i]);
    }
    free(argv_copy);

    return ret;
  }

 private:
  // clang-format off
  void AddCoreOptions() {
    options_.add_options("Core")
        ("h, help", "Print this help message.")
        ("version", "Print version number of BioDynaMo.")
        ("opencl", "Enable GPU acceleration through OpenCL.")
        ("cuda", "Enable GPU acceleration through CUDA.")
        ("v, verbose", "Verbose mode. Causes BioDynaMo to print debugging messages. Multiple "
         "-v options increases the verbosity. The maximum is 3.", value<bool>())
        ("r, restore", "Restores the simulation from the checkpoint found in FILE and "
         "continues simulation from that point.", value<string>()->default_value(""), "FILE")
        ("b, backup", "Periodically create full simulation backup to the specified file. "
         "NOTA BENE: File will be overriden if it exists.", value<string>()->default_value(""), "FILE")
        ("c, config", "The TOML configuration that should be used.", value<string>()->default_value(""), "FILE");
  }
  // clang-format on

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  void ExtractSimulationName(const char* path) {
    std::string s(path);
    auto pos = s.find_last_of("/");
    if (pos == std::string::npos) {
      sim_name_ = s;
    } else {
      sim_name_ = s.substr(pos + 1, s.length() - 1);
    }
  }

  void HandleCoreOptions(cxxopts::ParseResult& ret) {
    // Handle "help" argument
    if (ret.count("help")) {
      auto groups = options_.groups();
      auto it = std::find(groups.begin(), groups.end(), "Core");
      std::rotate(it, it + 1, groups.end());
      std::cout << options_.help(groups) << std::endl;
      exit(0);
    }

    if (ret.count("version")) {
      std::cout << "BioDynaMo Version: " << Version::String() << std::endl;
      exit(0);
    }

    // Handle "verbose" argument
    Int_t ll = kError;
    if (ret.count("verbose")) {
      auto verbosity = ret.count("verbose");

      switch (verbosity) {
        // case 0 can never occur; we wouldn't go into this if statement
        case 1:
          ll = kWarning;
          break;
        case 2:
          ll = kInfo;
          break;
        case 3:
          ll = kPrint;
          break;
        default:
          ll = kPrint;
          break;
      }
    }
    // Global variable of ROOT that determines verbosity of logging functions
    gErrorIgnoreLevel = ll;

// Handle "cuda" and "opencl" arguments
#ifdef USE_CUDA
    param->use_gpu_ = true;
#endif  // USE_CUDA

#ifdef USE_OPENCL
    param->use_gpu_ = true;
    param->use_opencl_ = true;
#endif  // USE_OPENCL
  }

  int argc_;
  const char** argv_;
  std::string sim_name_;
  cxxopts::Options options_;
};

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
