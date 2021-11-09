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

#include <json.hpp>

#include "core/multi_simulation/algorithm/algorithm.h"
#include "core/multi_simulation/algorithm/algorithm_registry.h"
#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/mpi_helper.h"
#include "core/multi_simulation/optimization_param.h"
#include "core/simulation.h"

using nlohmann::json;

namespace bdm {
namespace experimental {

/// Perform an exhaustive sweep across specified parameters
struct ParameterSweep : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(
      const std::function<void(Param*, TimeSeries*)>& send_params_to_worker,
      Param* default_params) override {
    auto sweeping_params = default_params->Get<OptimizationParam>()->params;

    if (sweeping_params.empty()) {
      Log::Error("ParameterSweep", "No sweeping parameters found!");
      return;
    }

    DynamicNestedLoop(sweeping_params, [&](const std::vector<uint32_t>& slots) {
      json j_patch;

      int i = 0;
      for (auto* param : sweeping_params) {
        j_patch[param->GetGroupName()][param->GetParamName()] =
            param->GetValue(slots[i]);
        i++;
      }

      Param final_params = *default_params;
      final_params.MergeJsonPatch(j_patch.dump());

      send_params_to_worker(&final_params, nullptr);
    });
  };
};

BDM_REGISTER_ALGO(ParameterSweep);

}  // namespace experimental
}  // namespace bdm
