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

#ifndef CORE_UTIL_CPPTOML_H_
#define CORE_UTIL_CPPTOML_H_

#define BDM_ASSIGN_CONFIG_VALUE(variable, config_key)           \
  {                                                             \
    auto bdm_toml_val_ =                                        \
        config.at_path(config_key).value<decltype(variable)>(); \
    if (bdm_toml_val_) {                                        \
      variable = *bdm_toml_val_;                                \
    }                                                           \
  }

#define BDM_ASSIGN_CONFIG_DOUBLE3_VALUE(variable, config_key)         \
  {                                                                   \
    if (auto bdm_arr_ = config.at_path(config_key).as_array()) {      \
      if (bdm_arr_->size() == variable.size()) {                      \
        for (uint64_t i = 0; i < bdm_arr_->size(); i++) {             \
          if (auto v_ = (*bdm_arr_)[i].value<real_t>())               \
            variable[i] = *v_;                                        \
        }                                                             \
      } else {                                                        \
        Log::Fatal("toml parameter parsing",                          \
                   "An error occurred during parameter parsing of (", \
                   config_key, ". Array dimensions do not match");    \
      }                                                               \
    }                                                                 \
  }

#endif  // CORE_UTIL_CPPTOML_H_
