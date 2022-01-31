// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#define BDM_ASSIGN_CONFIG_DOUBLE3_VALUE(variable, config_key)          \
  {                                                                    \
    if (config->contains_qualified(config_key)) {                      \
      auto value = config->get_array_of<double>(config_key);           \
      if (value) {                                                     \
        auto vector = *value;                                          \
        if (vector.size() == variable.size()) {                        \
          for (uint64_t i = 0; i < vector.size(); i++) {               \
            variable[i] = vector[i];                                   \
          }                                                            \
        } else {                                                       \
          Log::Fatal("cpptoml parameter parsing",                      \
                     "An error occured during parameter parsing of (", \
                     config_key, ". Array dimensions do not match");   \
        }                                                              \
      }                                                                \
    }                                                                  \
  }

#endif  // CORE_UTIL_CPPTOML_H_
