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

#include <string>

namespace bdm {

/// struct holding the parsed command line option values
struct CommandLineOptions {
  std::string backup_file_ = "";
  std::string restore_file_ = "";
};

/// This function parses command line arguments using a default set of options
/// for simulations
CommandLineOptions DefaultSimulationOptionParser(int& argc,            // NOLINT
                                                 const char**& argv);  // NOLINT

}  // namespace bdm

#endif  // CORE_PARAM_COMMAND_LINE_OPTIONS_H_
