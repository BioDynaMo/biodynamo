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

#include <json.hpp>

#include "core/parallel_execution/algorithm/algorithm.h"
#include "core/parallel_execution/algorithm/algorithm_registry.h"
#include "core/parallel_execution/dynamic_loop.h"
#include "core/parallel_execution/optimization_param.h"
#include "core/parallel_execution/util.h"
#include "core/simulation.h"

using nlohmann::json;

namespace bdm {

/// Perform an exhaustive sweep across specified parameters
struct ParameterSweep : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(
      const std::function<void(Param*)>& send_params_to_worker) override {
    auto sweeping_params = opt_params_->params_;

    if (sweeping_params.empty()) {
      Log::Error("ParameterSweep", "No sweeping parameters found!");
      return;
    }

    DynamicNestedLoop(sweeping_params, [&](const std::vector<int>& slots) {
      json j_patch;

      int i = 0;
      for (auto* param : sweeping_params) {
        j_patch[param->GetModuleName()][param->GetParamName()] =
            param->GetValue(slots[i]);
        i++;
      }

      std::cout << "Applied patch:" << std::endl;
      std::cout << j_patch.dump(4) << std::endl;

      Param final_params = *default_params_;
      final_params.MergeJsonPatch(j_patch.dump());

      send_params_to_worker(&final_params);
    });
  };
};

BDM_REGISTER_ALGO(ParameterSweep);

}  // namespace bdm
