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
#include "core/multi_simulation/result_data.h"
#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/multi_simulation_manager.h"

using nlohmann::json;

namespace bdm {

inline double MSE(const std::vector<double>& v1,
                  const std::vector<double>& v2) {
  if (v1.size() != v2.size()) {
    Log::Fatal("MSE", "vectors must have same length");
  }
  double error = 0;
  for (size_t i = 0; i < v1.size(); ++i) {
    auto diff = v2[i] - v1[i];
    error += diff * diff;
  }
  return error / v1.size();
}

void CalcMean(const std::vector<ResultData>& results, ResultData* rmean) {
  rmean->time_ = results[0].time_;
  std::vector<double> tmp(results.size());

  // susceptible
  for (uint64_t i = 0; i < rmean->time_.size(); ++i) {
    for (uint64_t j = 0; j < results.size(); ++j) {
      tmp[j] = results[j].susceptible_[i];
    }
    rmean->susceptible_.push_back(TMath::Mean(tmp.begin(), tmp.end()));
  }

  // infected
  for (uint64_t i = 0; i < rmean->time_.size(); ++i) {
    for (uint64_t j = 0; j < results.size(); ++j) {
      tmp[j] = results[j].infected_[i];
    }
    rmean->infected_.push_back(TMath::Mean(tmp.begin(), tmp.end()));
  }

  // recovered
  for (uint64_t i = 0; i < rmean->time_.size(); ++i) {
    for (uint64_t j = 0; j < results.size(); ++j) {
      tmp[j] = results[j].recovered_[i];
    }
    rmean->recovered_.push_back(TMath::Mean(tmp.begin(), tmp.end()));
  }
}

double Experiment(
    const std::function<void(Param*, ResultData*)>& dispatch_to_worker,
    Param* param, size_t iterations) {
  std::vector<ResultData> results(iterations);
  for (size_t i = 0; i < iterations; i++) {
    dispatch_to_worker(param, &results[i]);
  }

  // Compute mean error
  ResultData mean;
  CalcMean(results, &mean);

  // TODO: Extract analytical / experimental values

  double mse =
      MSE(std::vector<double>(mean.susceptible_.size()), mean.susceptible_) +
      MSE(std::vector<double>(mean.susceptible_.size()), mean.infected_);

  return mse;
}

struct ParticleSwarm : public Algorithm {
  BDM_ALGO_HEADER();

  void operator()(const std::function<void(Param*, ResultData*)>&
                      dispatch_to_worker) override {
    Param param = *default_params_;

    // The number of times to run an experiment
    size_t iterations = 1;

    // Initial values of the free parameters that we want to optimize
    arma::vec inout({0, 0, 0});

    // Set the bounds of the free params
    optim::algo_settings_t settings;
    settings.vals_bound = true;
    settings.lower_bounds = arma::vec({0.001, 5, 2});
    settings.upper_bounds =
        arma::vec({1, param.max_bound / 2, param.max_bound / 2});

    // The fitting function (i.e. calling a simulation with a paramset)
    auto fit = [&](const arma::vec& free_params, arma::vec* grad_out,
                   void* opt_data) {
      // Merge the free param value sinto the Param object that will be sent to
      // the worker
      // TODO

      double mse = Experiment(dispatch_to_worker, &param, iterations);
      std::cout << " MSE " << mse << " inout " << free_params(0) << " - "
                << free_params(1) << " - " << free_params(2) << std::endl
                << std::endl;
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
