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
#ifndef SIMULATION_IMPLEMENTATION_H_
#define SIMULATION_IMPLEMENTATION_H_

#include <sstream>
#include <cmath>
#include <omp.h>
#include "resource_manager.h"
#include "grid.h"
#include "scheduler.h"
#include "version.h"
#include "param.h"
#include "command_line_options.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

template <typename T>
std::atomic<uint64_t> Simulation<T>::counter_;

template <typename T>
Simulation<T>* Simulation<T>::active_ = nullptr;

template <typename T>
Simulation<T>* Simulation<T>::GetActive() {
  return active_;
}

template <typename T>
Simulation<T>::Simulation(TRootIOCtor*) {}

template <typename T>
Simulation<T>::Simulation(int argc, const char** argv) {
  Initialize(argc, argv);
}

template <typename T>
Simulation<T>::Simulation(const std::string& simulation_name) {
  const char* argv[1] = {simulation_name.c_str()};
  Initialize(1, argv);
}

template <typename T>
void Simulation<T>::Restore(Simulation<T>&& restored) {
  // random_
  if(random_.size() != restored.random_.size()) {
    Log::Warning("Simulation", "The restore file (",  param_->restore_file_,
    ") was run with a different number of threads. Can't restore complete random number generator state." );
    uint64_t min = std::min(random_.size(), restored.random_.size());
    for (uint64_t i = 0; i < min; i++) {
      *(random_[i]) = *(restored.random_[i]);
    }
  } else {
    for (uint64_t i = 0; i < random_.size(); i++) {
      *(random_[i]) = *(restored.random_[i]);
    }
  }

  // param and rm
  *param_ = *restored.param_;
  *rm_ = std::move(*restored.rm_);

  // name_ and unique_name_
  InitializeUniqueName(restored.name_);
}

template <typename T>
Simulation<T>::~Simulation() {
  Simulation<>* tmp = nullptr;
  if (active_ != this) {
    tmp = active_;
  }
  active_ = this;

  delete rm_;
  delete grid_;
  delete scheduler_;
  delete param_;
  for (auto* r : random_) {
    delete r;
  }
  active_ = tmp;
}

template <typename T>
void Simulation<T>::Activate() {
  active_ = this;
}

template <typename T>
ResourceManager<T>* Simulation<T>::GetResourceManager() { return rm_; }

template <typename T>
Param* Simulation<T>::GetParam() { return param_; }

template <typename T>
Grid<Simulation<T>>* Simulation<T>::GetGrid() { return grid_; }

template <typename T>
Scheduler<Simulation<T>>* Simulation<T>::GetScheduler() { return scheduler_; }

template <typename T>
Random* Simulation<T>::GetRandom() { return random_[omp_get_thread_num()]; }

template <typename T>
const std::string& Simulation<T>::GetUniqueName() const {
  return unique_name_;
}

template <typename T>
void Simulation<T>::ReplaceScheduler(Scheduler<Simulation>* scheduler) {
  delete scheduler_;
  scheduler_ = scheduler;
}

template <typename T>
void Simulation<T>::Initialize(int argc, const char** argv) {
  id_ = counter_++;
  Activate();
  InitializeRuntimeParams(argc, argv);
  InitializeUniqueName(ExtractSimulationName(argv[0]));
  InitializeMembers();
}

template <typename T>
template <typename TResourceManager,
          typename TGrid,
          typename TScheduler>
void Simulation<T>::InitializeMembers() {
  random_.resize(omp_get_max_threads());
  for (uint64_t i = 0; i < random_.size(); i++) {
    random_[i] = new Random();
  }
  rm_ = new TResourceManager();
  grid_ = new TGrid();
  scheduler_ = new TScheduler();
}

template <typename T>
void Simulation<T>::InitializeRuntimeParams(int argc, const char** argv) {
  param_ = new Param();

  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ", Version::String());

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDM_CMAKE_DIR") == nullptr) {
    Log::Fatal("Simulation::InitializeRuntimeParams",
               "The BioDynaMo environment is not set up correctly. Please call "
               "$use_biodynamo "
               "and retry this command.");
  }

  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  constexpr auto kConfigFile = "bdm.toml";
  constexpr auto kConfigFileParentDir = "../bdm.toml";
  if (FileExists(kConfigFile)) {
    auto config = cpptoml::parse_file(kConfigFile);
    param_->AssignFromConfig(config);
  } else if (FileExists(kConfigFileParentDir)) {
    auto config = cpptoml::parse_file(kConfigFileParentDir);
    param_->AssignFromConfig(config);
  } else {
    Log::Warning("Simulation::InitializeRuntimeParams",
                 "Config file %s not found in `.` or `../` directory.",
                 kConfigFile);
  }
  if (options.backup_file_ != "") {
    param_->backup_file_ = options.backup_file_;
    param_->restore_file_ = options.restore_file_;
  }
}

template <typename T>
void Simulation<T>::InitializeUniqueName(const std::string& simulation_name) {
  name_ = simulation_name;
  std::stringstream stream;
  stream << name_;
  if (id_ > 0) {
    stream << id_;
  }
  unique_name_ = stream.str();
}


template <typename T>
std::string Simulation<T>::ExtractSimulationName(const char* path) {
  std::string s(path);
  auto pos = s.find_last_of("/");
  if (pos == std::string::npos) {
    return s;
  } else {
    return s.substr(pos + 1, s.length() - 1);
  }
}

}  // namespace bdm

#endif  // SIMULATION_IMPLEMENTATION_H_
