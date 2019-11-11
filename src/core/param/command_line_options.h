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

#include "core/simulation.h"

#include <iostream>
#include <string>

namespace bdm {

/// Class holding the parsed command line options
class CommandLineOptions {
public:
  CommandLineOptions() : options_("", " -- BioDynaMo command line options\n") {
    AddCoreArguments();
  }

  // clang-format off
  void AddCoreArguments() {
    options_.add_options("Core")
        ("h, help", "Prints this help message")
        ("opencl", "Enable GPU acceleration through OpenCL.")
        ("cuda", "Enable GPU acceleration through CUDA.")
        ("v, verbose", "Verbose mode. Causes BioDynaMo to print debugging messages. Multiple "
         "-v options increases the verbosity. The maximum is 3.", value<bool>())
        ("r, restore", "Restores the simulation from the checkpoint found in FILE and "
         "continues simulation from that point.", value<string>(), "FILE")
        ("b, backup", "Periodically create full simulation backup to the specified file. "
         "NOTA BENE: File will be overriden if it exists", value<string>(), "FILE")
        ("c, config", "The TOML configuration that should be used.", value<string>(), "FILE");
  }
  // clang-format on

  // Add an extra command line option
  cxxopts::OptionAdder AddOption(std::string group = "") {
    return cxxopts::OptionAdder(options_, std::move(group));
  }

  // Parse the given command line arguments
  cxxopts::ParseResult Parse(int argc, char **argv) {
    auto ret = options_.parse(argc, argv);
    HandleCoreOptions(ret);
    return ret;
  }

  void HandleCoreOptions(cxxopts::ParseResult ret&) {
    auto* param = Simulation::GetActive()->GetParam();

    // Handle "help" argument
    if (ret.count("help")) {
      std::cout << options_.help({"", "Core"}) << std::endl;
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
      // Global variable of ROOT that determines verbosity of logging functions
      gErrorIgnoreLevel = ll;
    }

    // Handle "cuda" and "opencl" arguments
#ifdef USE_CUDA
    param->use_gpu_ = true;
#endif  // USE_CUDA

#ifdef USE_OPENCL
    param->use_gpu_ = true;
    param->use_opencl_ = true;
#endif  // USE_OPENCL
  }


private:
  cxxopts::Options options_;
};

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
