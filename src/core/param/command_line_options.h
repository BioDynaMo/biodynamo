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

#include <iostream>
#include <string>

namespace bdm {

using cxxopts::value;
using std::string;

/// Class to contain and parse command line options
class CommandLineOptions {
 public:
  static CommandLineOptions* GetInstance(int argc, const char** argv) {
    static CommandLineOptions clo(argc, argv);
    return &clo;
  }

  /// Add an extra command line option
  cxxopts::OptionAdder AddOption(string group = "") {
    return cxxopts::OptionAdder(options_, std::move(group));
  }

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
  // This constructor is only called on the first invocation to
  // `CommandLineOptions::GetInstance()`
  CommandLineOptions(int argc, const char** argv)
      : argc_(argc),
        argv_(argv),
        options_(argv[0], " -- BioDynaMo command line options\n") {
    AddCoreOptions();
  }

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

  void HandleCoreOptions(cxxopts::ParseResult& ret) {
    // Handle "help" argument
    if (ret.count("help")) {
      std::cout << options_.help({"", "Core"}) << std::endl;
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
  cxxopts::Options options_;
};

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
