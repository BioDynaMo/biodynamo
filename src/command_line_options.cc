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

#include "command_line_options.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "OptionParser.h"
#include "log.h"

namespace bdm {

CommandLineOptions DefaultSimulationOptionParser(
    int& argc,             // NOLINT
    const char**& argv) {  // NOLINT
  auto binary_name = argv[0];

  // NB: parser does not work without these statement
  // skip program name argv[0] if present
  argc -= (argc > 0);
  argv += (argc > 0);

  enum OptionIndex {
    kUnknown,
    kVerbose,
    kRestoreFilename,
    kBackupFilename,
    kHelp
  };
  enum OptionTypes { kNoType, kString };

  std::stringstream simulation_usage_stream;
  simulation_usage_stream << "Start " << binary_name << " simulation.\n";
  simulation_usage_stream << "Usage: " << binary_name << " [options]\n\n";
  simulation_usage_stream << "Options:\n";

  const char* simulation_usage = simulation_usage_stream.str().c_str();

  const char* verbose_usage =
      "-v, --verbose\n"
      "    Verbose mode. Causes BioDynaMo to print debugging messages.\n"
      "    Multiple -v options increases the verbosity. The maximum is 3.\n";

  const char* restore_file_usage =
      "-r, --restore filename\n"
      "    Restores the simulation from the checkpoint found in filename and\n"
      "    continues simulation from that point.\n";

  const char* backup_file_usage =
      "-b, --backup filename\n"
      "    Periodically create full simulation backup to the specified file\n"
      "    NOTA BENE: File will be overriden if it exists\n";

  // clang-format off
  const ROOT::option::Descriptor simulation_usage_descriptor[] = {
      {
        kUnknown,
        kNoType,
        "", "",
        ROOT::option::Arg::None,
        simulation_usage
      },

      {
        kVerbose,
        kNoType,
        "v", "",
        ROOT::option::Arg::None,
        verbose_usage
      },

      {
        kRestoreFilename,
        kString,
        "r", "restore",
        ROOT::option::FullArg::Required,
        restore_file_usage
      },

      {
        kBackupFilename,
        kString,
        "b", "backup",
        ROOT::option::FullArg::Required,
        backup_file_usage
      },

      {
        kHelp,
        kNoType,
        "h", "help",
        ROOT::option::Arg::None,
        "--help\n    Print usage and exit.\n"
      },
      {0, 0, 0, 0, 0, 0}
    };
  // clang-format on

  ROOT::option::Stats stats(simulation_usage_descriptor, argc, argv);
  ROOT::option::Option options[stats.options_max], buffer[stats.buffer_max];
  ROOT::option::Parser parse(simulation_usage_descriptor, argc, argv, options,
                             buffer);

  if (parse.error()) {
    Log::Error("CommandLineOptions", "Argument parsing error!\n");
    ROOT::option::printUsage(std::cout, simulation_usage_descriptor);
    exit(1);
  }

  // Print help if needed
  if (options[kHelp]) {
    ROOT::option::printUsage(std::cout, simulation_usage_descriptor);
    exit(0);
  }

  CommandLineOptions cl_options;

  Int_t ll = kError;
  if (options[kVerbose]) {
    int verbosity = options[kVerbose].count();
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

  if (options[kRestoreFilename]) {
    cl_options.restore_file_ = options[kRestoreFilename].arg;
    // TODO(lukas) check file ending
  }

  if (options[kBackupFilename]) {
    cl_options.backup_file_ = options[kBackupFilename].arg;
    // TODO(lukas) check file ending
  }

  return cl_options;
}

}  // namespace bdm
