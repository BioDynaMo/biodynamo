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

#ifndef CORE_UTIL_CPPTOML_H_
#define CORE_UTIL_CPPTOML_H_

#define BDM_ASSIGN_CONFIG_VALUE(variable, config_key)                        \
  {                                                                          \
    if (config->contains_qualified(config_key)) {                            \
      auto value = config->get_qualified_as<decltype(variable)>(config_key); \
      if (value) {                                                           \
        variable = *value;                                                   \
      }                                                                      \
    }                                                                        \
  }

#endif  // CORE_UTIL_CPPTOML_H_
