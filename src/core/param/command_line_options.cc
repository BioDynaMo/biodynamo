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

#include "core/param/command_line_options.h"
#include <TEnv.h>
#include <utility>
#include "core/param/param.h"
#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {

using cxxopts::value;
using std::string;

CommandLineOptions::CommandLineOptions(int argc, const char** argv)
    : argc_(argc),
      argv_(argv),
      options_(argv[0], " -- BioDynaMo command line options\n") {
  AddCoreOptions();
  ExtractSimulationName(argv[0]);
}

CommandLineOptions::~CommandLineOptions() {
  if (parser_) {
    delete parser_;
    parser_ = nullptr;
  }
}

cxxopts::OptionAdder CommandLineOptions::AddOption(string group) {
  if (parser_ != nullptr) {
    Log::Fatal("CommandLineOptions::AddOption",
               "Please add all your command line options before:\n  1) Using "
               "the Get() method.\n  2) Creating a Simulation object.");
  }
  return cxxopts::OptionAdder(options_, std::move(group));
}

std::string CommandLineOptions::GetSimulationName() { return sim_name_; }

/// Parse the given command line arguments
void CommandLineOptions::Parse() {
  // Make a non-const deep copy of argv
  char** argv_copy = (char**)malloc((argc_ + 1) * sizeof(char*));
  int argc_copy = argc_;
  for (int i = 0; i < argc_; ++i) {
    size_t length = strlen(argv_[i]) + 1;
    argv_copy[i] = (char*)malloc(length);
    memcpy(argv_copy[i], argv_[i], length);
  }
  argv_copy[argc_] = NULL;

  // Perform parsing (consumes argc_copy and argv_copy)
  if (parser_) {
    delete parser_;
    parser_ = nullptr;
  }

  try {
    parser_ = new cxxopts::ParseResult(options_.parse(argc_copy, argv_copy));
  } catch (const cxxopts::option_not_exists_exception&) {
    Log::Fatal("CommandLineOptions::ParseResult",
               "Please add all your command line options before:\n  1) Using "
               "the CommandLineOptions::Get() method.\n  2) Creating a "
               "Simulation object.");
  }

  if (first_parse_) {
    // Perform operations on core command line options
    HandleCoreOptions();
    first_parse_ = false;
  }

  // free memory
  for (int i = 0; i < argc_; ++i) {
    free(argv_copy[i]);
  }
  free(argv_copy);
}

bool CommandLineOptions::IsSet(std::string option) {
  if (parser_ == nullptr) {
    this->Parse();
  }
  return parser_->count(option) == 0 ? false : true;
}

// clang-format off
void CommandLineOptions::AddCoreOptions() {
  options_.add_options("Core")
    ("h, help", "Print this help message.")
    ("version", "Print version number of BioDynaMo.")
    ("data", "Data file to drive optimization feedback loop with (csv supported only).", value<string>()->default_value(""), "FILE.csv")
    ("opencl", "Enable GPU acceleration through OpenCL.")
    ("cuda", "Enable GPU acceleration through CUDA.")
    ("visualize", "Enable exporting of visualization.")
    ("vis-frequency", "Set the frequency of exporting the visualization.", value<uint32_t>()->default_value("10"), "FREQ")
    ("v, verbose", "Verbose mode. Causes BioDynaMo to print debugging messages. Multiple "
      "-v options increases the verbosity. The maximum is 3.", value<bool>())
    ("r, restore", "Restores the simulation from the checkpoint found in FILE and "
      "continues simulation from that point.", value<string>()->default_value(""), "FILE")
    ("b, backup", "Periodically create full simulation backup to the specified file. "
      "NOTA BENE: File will be overriden if it exists.", value<string>()->default_value(""), "FILE")
    ("c, config", "The TOML or JSON configuration that should be used. The JSON file must be in JSON merge patch format (https://tools.ietf.org/html/rfc7386). This option can be used multiple times.", value<std::vector<string>>()->default_value(""), "FILE")
    ("inline-config", "JSON configuration string passed directly on the command line. Overwrites values specified in config file.  The JSON string must be in JSON merge patch format (https://tools.ietf.org/html/rfc7386). This option can be used multiple times.", value<std::vector<string>>()->default_value(""), "JSON_STRING")
    ("output-default-json", "Prints a JSON string with all parameters and their default values and exits.")
    ("toml-to-json", "Converts a TOML file to a JSON patch. After printing the JSON patch the application will exit.", value<string>()->default_value(""), "TOML_FILE");
}
// clang-format on

void CommandLineOptions::ExtractSimulationName(const char* path) {
  string s(path);
  auto pos = s.find_last_of("/");
  if (pos == std::string::npos) {
    sim_name_ = s;
  } else {
    sim_name_ = s.substr(pos + 1, s.length() - 1);
  }
}

void CommandLineOptions::HandleCoreOptions() {
  // Handle "help" argument
  if (parser_->count("help")) {
    auto groups = options_.groups();
    auto it = std::find(groups.begin(), groups.end(), "Core");
    std::rotate(it, it + 1, groups.end());
    std::cout << options_.help(groups) << std::endl;
    exit(0);
  }

  if (IsSet("version")) {
    std::cout << "BioDynaMo Version: " << Version::String() << std::endl;
    exit(0);
  }

  // Handle "verbose" argument
  // If set in etc/bdm.rootrc use that value, command line argument will
  // override it
  Int_t ll = kWarning;
  TString slevel = "Warning";
  TEnvRec* rec = gEnv->Lookup("Root.ErrorIgnoreLevel");
  if (rec) {
    if (rec->GetLevel() == kEnvUser)
      slevel = rec->GetValue();
  }
  if (!slevel.CompareTo("Print", TString::kIgnoreCase))
    ll = kPrint;
  else if (!slevel.CompareTo("Info", TString::kIgnoreCase))
    ll = kInfo;
  else if (!slevel.CompareTo("Warning", TString::kIgnoreCase))
    ll = kWarning;
  else if (!slevel.CompareTo("Error", TString::kIgnoreCase))
    ll = kError;

  if (IsSet("verbose")) {
    auto verbosity = parser_->count("verbose");

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
  if (IsSet("cuda")) {
    param->use_gpu_ = true;
  }
#endif  // USE_CUDA

#ifdef USE_OPENCL
  if (IsSet("opencl")) {
    param->use_gpu_ = true;
    param->use_opencl_ = true;
  }
#endif  // USE_OPENCL

  if (parser_->count("output-default-json")) {
    Param param;
    std::cout << "Below you can find a JSON string with all available "
                 "parameters and their default values.\n"
              << "Have a look at https://biodynamo.org/bioapi/ for more "
                 "details about each parameter."
              << std::endl;
    std::cout << param.ToJsonString() << std::endl;
    exit(0);
  }

  auto toml_file = (*parser_)["toml-to-json"].as<std::string>();
  if (toml_file != "") {
    if (!FileExists(toml_file)) {
      Log::Fatal("CommandLineOptions::HandleCoreOptions",
                 "Specified TOML file (", toml_file, ") does not exist.");
    }
    auto toml = cpptoml::parse_file(toml_file);
    Param param;
    param.AssignFromConfig(toml);
    std::cout << param.ToJsonString() << std::endl;
    exit(0);
  }
}

std::ostream& operator<<(std::ostream& os, const CommandLineOptions& clo) {
  for (int i = 0; i < clo.argc_; ++i) {
    os << clo.argv_[i] << " ";
  }
  return os;
}

}  // namespace bdm
