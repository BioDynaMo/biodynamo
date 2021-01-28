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

#include "neuroscience/param.h"
#include "core/util/cpptoml.h"

namespace bdm {
namespace neuroscience {

const ParamGroupUid Param::kUid = ParamGroupUidGenerator::Get()->NewUid();

void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_actual_length,
                          "neuroscience.neurite_default_actual_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_density,
                          "neuroscience.neurite_default_density");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_diameter,
                          "neuroscience.neurite_default_diameter");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_spring_constant,
                          "neuroscience.neurite_default_spring_constant");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_adherence,
                          "neuroscience.neurite_default_adherence");
  BDM_ASSIGN_CONFIG_VALUE(neurite_default_tension,
                          "neuroscience.neurite_default_tension");
  BDM_ASSIGN_CONFIG_VALUE(neurite_min_length,
                          "neuroscience.neurite_min_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_max_length,
                          "neuroscience.neurite_max_length");
  BDM_ASSIGN_CONFIG_VALUE(neurite_minimial_bifurcation_length,
                          "neuroscience.neurite_minimial_bifurcation_length");
}

}  // namespace neuroscience
}  // namespace bdm
