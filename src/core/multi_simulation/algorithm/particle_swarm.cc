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
#include "optim.hpp"

#include "core/multi_simulation/algorithm/algorithm.h"
#include "core/multi_simulation/algorithm/algorithm_registry.h"
#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/experiment.h"
#include "core/multi_simulation/multi_simulation_manager.h"

using nlohmann::json;

namespace bdm {

struct ParticleSwarm : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(
      const std::function<void(Param*, TimeSeries*)>& dispatch_to_worker,
      Param* default_params) override {
    Param new_param = *default_params;
    OptimizationParam* opt_params = new_param.Get<OptimizationParam>();

    // The number of times to run an experiment
    int repetition = opt_params->repetition_;

    // Initial values and the bounds of the free parameters that we want to
    // optimize
    std::vector<std::string> param_names;
    std::vector<double> init_vals;
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    for (auto* el : opt_params->params_) {
      auto* opt_param = dynamic_cast<ParticleSwarmParam*>(el);
      if (!opt_param) {
        Log::Error(
            "ParticleSwarm::operator()",
            "Encountered non-ParticleSwarmParam type optimization parameter: ",
            el->GetParamName());
        continue;
      }
      param_names.push_back(opt_param->GetParamName());
      init_vals.push_back(opt_param->initial_value);
      lower_bounds.push_back(opt_param->lower_bound);
      upper_bounds.push_back(opt_param->upper_bound);
    }
    arma::vec inout(init_vals);
    optim::algo_settings_t settings;
    settings.vals_bound = true;
    settings.lower_bounds = arma::vec(lower_bounds);
    settings.upper_bounds = arma::vec(upper_bounds);

    // The fitting function (i.e. calling a simulation with a paramset)
    auto fit = [&](const arma::vec& free_params, arma::vec* grad_out,
                   void* opt_data) {
      // Merge the free param values into the Param object that will be sent to
      // the worker
      json j_patch;
      int i = 0;
      for (auto* opt_param : opt_params->params_) {
        j_patch[opt_param->GetGroupName()][opt_param->GetParamName()] =
            free_params[i];
        i++;
      }
      new_param.MergeJsonPatch(j_patch.dump());

      double mse = Experiment(dispatch_to_worker, &new_param, repetition);
      std::cout << " MSE " << mse << " inout " << free_params << std::endl;
      return mse;
    };

    // Call the optimization routine
    if (!optim::pso(inout, fit, nullptr, settings)) {
      Log::Fatal("", "Optimization algorithm didn't complete successfully.");
    }
  };
};

BDM_REGISTER_ALGO(ParticleSwarm);

}  // namespace bdm
