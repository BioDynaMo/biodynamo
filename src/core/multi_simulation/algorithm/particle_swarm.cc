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
#include <omp.h>
#include "optim.hpp"

#include "core/multi_simulation/algorithm/algorithm.h"
#include "core/multi_simulation/algorithm/algorithm_registry.h"
#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/experiment.h"
#include "core/multi_simulation/multi_simulation_manager.h"
#include "core/multi_simulation/optimization_param_type/particle_swarm_param.h"

using nlohmann::json;

namespace bdm {
namespace experimental {

/// Implements the particle swarm optimization algorithm
/// For more info: https://www.kthohr.com/optimlib_docs_pso.html
struct ParticleSwarm : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(Functor<void, Param*, TimeSeries*>& dispatch_experiment,
                  Param* default_params) override {
    OptimizationParam* opt_params = default_params->Get<OptimizationParam>();

    // The number of times to run an experiment
    int repetition = opt_params->repetition;

    // Initial values and the bounds of the free parameters that we want to
    // optimize
    std::vector<std::string> param_names;
    std::vector<double> init_vals;
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;

    if (opt_params->params.empty()) {
      Log::Fatal("ParticleSwarm::operator()",
                 "No optimization parameters were selected. Please check your "
                 "parameter configuration.");
    }

    for (auto* el : opt_params->params) {
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
    settings.pso_n_gen = opt_params->max_iterations;

    auto max_it = settings.pso_n_gen;
    int iteration = 0;
    double min_mse = 1e9;
    json best_params;
    double prev_mse = 1.0;

    // The fitting function (i.e. calling a simulation with a paramset)
    // Anything inside this function should be thread-safe
    auto fit = [=, &dispatch_experiment, &iteration, &prev_mse, &min_mse,
                &best_params](const arma::vec& free_params, arma::vec* grad_out,
                              void* opt_data) {
      Param new_param = *default_params;

      std::cout << "iteration (" << iteration << "/" << max_it << ")"
                << std::endl;

      // Bug: on the rare occasion that we get NaN values back from optim:pso,
      // we should ignore it and return the previously obtained error
      for (auto& p : free_params) {
        if (std::isnan(p)) {
          return prev_mse + 0.005;
        }
      }

      // Merge the free param values into the Param object that will be sent to
      // the worker
      json j_patch;
      int i = 0;
      std::cout << "FP: " << free_params << std::endl;
      for (auto* opt_param : opt_params->params) {
        j_patch[opt_param->GetGroupName()][opt_param->GetParamName()] =
            free_params[i];
        i++;
      }

      std::cout << j_patch << std::endl;

      new_param.MergeJsonPatch(j_patch.dump());

      double mse = Experiment(dispatch_experiment, repetition, &new_param);
      std::cout << " MSE " << mse << " inout " << free_params << std::endl;
      // #pragma omp critical
      {
        iteration++;
        prev_mse = mse;
        // Check if the current error is smaller than the previously smallest
        // error If it is, then we save the corresponding parameters as the next
        // best set of parameters
        if (mse < min_mse) {
          min_mse = mse;
          best_params = j_patch;
        }
      }
      return mse;
    };

    // Call the optimization routine
    if (!optim::pso(inout, fit, nullptr, settings)) {
      Log::Fatal("", "Optimization algorithm didn't complete successfully.");
    }

    std::cout << "Best params = " << best_params << std::endl;
  };
};

BDM_REGISTER_ALGO(ParticleSwarm);

}  // namespace experimental
}  // namespace bdm
