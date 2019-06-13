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

#include "neuroscience/param.h"
#include "core/util/cpptoml.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

const ModuleParamUid Param::kUid = ModuleParamUidGenerator::Get()->NewUid();

ModuleParam* Param::GetCopy() const { return new Param(*this); }

ModuleParamUid Param::GetUid() const { return kUid; }

void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_actual_length_,
                          "neuroscience.neurite_default_actual_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_density_,
                          "neuroscience.neurite_default_density");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_diameter_,
                          "neuroscience.neurite_default_diameter");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_spring_constant_,
                          "neuroscience.neurite_default_spring_constant");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_adherence_,
                          "neuroscience.neurite_default_adherence");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_tension_,
                          "neuroscience.neurite_default_tension");
  BDM_ASSIGN_CONFIG_VALUE(neurite_min_length_,
                          "neuroscience.neurite_min_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_max_length_,
                          "neuroscience.neurite_max_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_minimial_bifurcation_length_,
                          "neuroscience.neurite_minimial_bifurcation_length");
}

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
