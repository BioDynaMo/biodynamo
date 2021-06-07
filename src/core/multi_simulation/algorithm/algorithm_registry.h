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

#ifndef CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_REGISTRY_H_
#define CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_REGISTRY_H_

#include <unordered_map>

#include "core/multi_simulation/algorithm/algorithm.h"
#include "core/multi_simulation/optimization_param.h"
#include "core/param/param.h"

namespace bdm {

class MultiSimulationManager;

struct AlgorithmRegistry {
  /// Singleton class - returns the static instance
  static AlgorithmRegistry *GetInstance();

  /// Gets the algorithm
  ///
  /// @param[in]  algo_name  The algorithm's name
  ///
  /// @return     The algorithm pointer
  ///
  Algorithm *GetAlgorithm(const std::string &algo_name);

  /// Adds an algorithm to the registry
  ///
  /// @param[in]  algo_name  The algorithm's name
  /// @param      algo       The algorithm
  ///
  /// @return     Returns true when the algorithm is successfully added to
  ///             registry
  ///
  bool AddAlgorithm(const std::string &algo_name, Algorithm *algo);

 private:
  /// The map containing the algorithms; accessible by their name
  std::unordered_map<std::string, Algorithm *> algorithms_;

  AlgorithmRegistry();
  ~AlgorithmRegistry();
};

#define BDM_REGISTER_ALGO(op) \
  bool op::registered_ =      \
      AlgorithmRegistry::GetInstance()->AddAlgorithm(#op, new op());

/// A convenient macro to hide some of the boilerplate code from the user in
/// implementing new algorithms
#define BDM_ALGO_HEADER() \
 private:                 \
  static bool registered_;

// Get Optimization Algorithm from registry
inline Algorithm *CreateOptimizationAlgorithm(OptimizationParam *opt_params) {
  // Check if the parameters are initialized correctly
  for (auto param : opt_params->params_) {
    param->Validate();
  }
  auto ret =
      AlgorithmRegistry::GetInstance()->GetAlgorithm(opt_params->algorithm_);
  if (ret == nullptr) {
    return ret;
  }
  return ret;
}

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_REGISTRY_H_
