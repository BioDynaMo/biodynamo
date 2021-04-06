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
#include "core/multi_simulation/multi_simulation_manager.h"

using nlohmann::json;

namespace bdm {

double GetExperimentalValue(const Param& final_params) {
  return 42.0;
}

/// TODO: currently ParameterSweep implementation
struct ParticleSwarm : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(
      const std::function<void(Param*)>& send_params_to_worker) override {
    for (size_t r = 0; r < msm_->data_->GetRowCount(); r++) {
      json j_patch;

      for (size_t c = 0; c < msm_->data_->GetColumnCount(); c++) {
        j_patch["bdm::SimParam"][msm_->data_->GetColumnName(c)] =
            msm_->data_->GetCell<double>(c, r);
      }

      Param final_params = *default_params_;
      // std::cout << j_patch.dump() << std::endl;
      final_params.MergeJsonPatch(j_patch.dump());

      // Extract the appropriate expected experimental value based on the
      // initial parameters
      json exp_data_patch;
      exp_data_patch["bdm::OptimizationParam"]["expected_val"] =
          GetExperimentalValue(final_params);

      final_params.MergeJsonPatch(exp_data_patch.dump());
      send_params_to_worker(&final_params);
    }
  };
};

BDM_REGISTER_ALGO(ParticleSwarm);

}  // namespace bdm
