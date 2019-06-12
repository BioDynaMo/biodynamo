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

#ifndef CORE_PARAM_CUSTOM_CLO_PARSER_H_
#define CORE_PARAM_CUSTOM_CLO_PARSER_H_

#include <sstream>
#include <stdexcept>
#include <string>

#include "core/util/string.h"

namespace bdm {

/// Parses custom command line options in the form of: `--parameter=value`
struct CustomCLOParser {
  CustomCLOParser(int argc, const char** argv) : argc_(argc), argv_(argv) {}

  /// Function returns the value for the given command line argument.
  /// A default value is returned if the argument was not specified. In this
  /// case `allow_default` must be true.
  template <typename T>
  T GetValue(const std::string& parameter, bool allow_default = false,
             T default_value = T()) {
    for (int i = 0; i < argc_; i++) {
      std::string arg(argv_[i]);
      std::string prefix = Concat("--", parameter, "=");
      if (!arg.compare(0, prefix.size(), prefix)) {
        T t;
        std::istringstream str(arg.substr(prefix.size()).c_str());
        str >> t;
        return t;
      }
    }
    if (allow_default) {
      return default_value;
    }

    throw std::logic_error(Concat("Could not find parameter '", parameter,
                                  "' and default value has been disabled"));
  }

  int argc_;
  const char** argv_;
};

}  // namespace bdm

#endif  // CORE_PARAM_CUSTOM_CLO_PARSER_H_
