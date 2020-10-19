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

#include "core/parallel_execution/algorithm/algorithm_registry.h"
#include "core/util/log.h"

namespace bdm {

AlgorithmRegistry::~AlgorithmRegistry() {
  for (auto &pair : algorithms_) {
    delete pair.second;
  }
}

AlgorithmRegistry *AlgorithmRegistry::GetInstance() {
  static AlgorithmRegistry algorithm_registry;
  return &algorithm_registry;
}

Algorithm *AlgorithmRegistry::GetAlgorithm(const std::string &algo_name) {
  if (algo_name.empty()) {
    Log::Warning("AlgorithmRegistry::GetAlgorithm",
               "No algorithm name defined in parameter configuration.");
    return nullptr;
  }
  auto search = algorithms_.find(algo_name);
  if (search == algorithms_.end()) {
    std::string msg = "Algorithm not found in registry: " + algo_name;
    Log::Fatal("AlgorithmRegistry::GetAlgorithm", msg);
  }
  return search->second;
}

bool AlgorithmRegistry::AddAlgorithm(const std::string &algo_name,
                                     Algorithm *algo) {
  auto algo_it = algorithms_.find(algo_name);
  // If algorithm doesn't exist yet, register the new algorithm under given name
  if (algo_it == algorithms_.end()) {
    algorithms_.insert(std::make_pair(algo_name, algo));
  }
  return true;
}

AlgorithmRegistry::AlgorithmRegistry() {}

}  // namespace bdm
