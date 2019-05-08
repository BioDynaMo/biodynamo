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

#ifndef NEUROSCIENCE_PARAM_H_
#define NEUROSCIENCE_PARAM_H_

#include <cinttypes>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/param/module_param.h"
#include "core/util/root.h"
#include "cpptoml/cpptoml.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

struct Param : public ModuleParam {
  static const ModuleParamUid kUid;

  ModuleParam* GetCopy() const override;

  ModuleParamUid GetUid() const override;

  /// Default actual length value of a neurite.\n
  /// Default value: `1.0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_actual_length = 1.0
  double neurite_default_actual_length_ = 1.0;

  /// Default density value of a neurite.\n
  /// Default value: `1.0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_density = 1.0
  double neurite_default_density_ = 1.0;

  /// Default diameter value of a neurite.\n
  /// Default value: `1.0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_diameter = 1.0
  double neurite_default_diameter_ = 1.0;

  /// Default spring constant value of a neurite.\n
  /// Default value: `10`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_spring_constant = 10
  double neurite_default_spring_constant_ = 10;

  /// Default adherence value of a neurite.\n
  /// Default value: `0.1`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_adherence = 0.1
  double neurite_default_adherence_ = 0.1;

  /// Default tension value of a neurite.\n
  /// Default value: `0.0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_default_tension = 0.0
  double neurite_default_tension_ = 0.0;

  /// Minimum allowed length of a neurite element.\n
  /// Default value: `2.0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_min_length = 2.0
  double neurite_min_length_ = 2.0;

  /// Maximum allowed length of a neurite element.\n
  /// Default value: `15`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_max_length = 15
  double neurite_max_length_ = 15;

  /// Minumum bifurcation length of a neurite element.\n
  /// If the length is below this threshold, bifurcation is not permitted.\n
  /// Default value: `0`\n
  /// TOML config file:
  ///
  ///     [neuroscience]
  ///     neurite_minimial_bifurcation_length = 0
  double neurite_minimial_bifurcation_length_ = 0;

 protected:
  /// Assign values from config file to variables
  void AssignFromConfig(const std::shared_ptr<cpptoml::table>&) override;

 private:
  BDM_CLASS_DEF_OVERRIDE(Param, 1);
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_PARAM_H_
