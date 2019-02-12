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

#include <cpptoml/cpptoml.h>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/grid.h"
#include "core/param/command_line_options.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/util/log.h"
#include "core/util/io.h"
#include "core/util/string.h"
#include "version.h"

namespace bdm {

/// Implementation for `Simulation`:
/// It must be separate to avoid circular dependencies.
/// It can't be defined in a source file, because it is templated.

std::atomic<uint64_t> Simulation::counter_;

Simulation* Simulation::active_ = nullptr;

Simulation* Simulation::GetActive() {
  return active_;
}

Simulation::Simulation(TRootIOCtor* p) {}

Simulation::Simulation(int argc, const char** argv)
    : Simulation(argc, argv, [](auto* param) {}) {}

Simulation::Simulation(const std::string& simulation_name)
    : Simulation(simulation_name, [](auto* param) {}) {}

Simulation::Simulation(int argc, const char** argv,
                          const std::function<void(Param*)>& set_param) {
  Initialize(argc, argv, set_param);
}

Simulation::Simulation(const std::string& simulation_name,
                          const std::function<void(Param*)>& set_param) {
  const char* argv[1] = {simulation_name.c_str()};
  Initialize(1, argv, set_param);
}

void Simulation::Restore(Simulation&& restored) {
  // random_
  if (random_.size() != restored.random_.size()) {
    Log::Warning("Simulation", "The restore file (", param_->restore_file_,
                 ") was run with a different number of threads. Can't restore "
                 "complete random number generator state.");
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
  param_->Restore(std::move(*restored.param_));
  restored.param_ = nullptr;
  *rm_ = std::move(*restored.rm_);
  restored.rm_ = nullptr;

  // name_ and unique_name_
  InitializeUniqueName(restored.name_);
  InitializeOutputDir();
}

Simulation::~Simulation() {
  Simulation* tmp = nullptr;
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
  for (auto* ectxt : exec_ctxt_) {
    delete ectxt;
  }
  active_ = tmp;
}

void Simulation::Activate() {
  active_ = this;
}

ResourceManager* Simulation::GetResourceManager() {
  return rm_;
}

const Param* Simulation::GetParam() const {
  return param_;
}

Grid* Simulation::GetGrid() {
  return grid_;
}

Scheduler* Simulation::GetScheduler() {
  return scheduler_;
}

Random* Simulation::GetRandom() {
  return random_[omp_get_thread_num()];
}

InPlaceExecutionContext* Simulation::GetExecutionContext() {
  return exec_ctxt_[omp_get_thread_num()];
}

std::vector<InPlaceExecutionContext*>& Simulation::GetAllExecCtxts() {
  return exec_ctxt_;
}

const std::string& Simulation::GetUniqueName() const {
  return unique_name_;
}

const std::string& Simulation::GetOutputDir() const {
  return output_dir_;
}

void Simulation::ReplaceScheduler(Scheduler* scheduler) {
  delete scheduler_;
  scheduler_ = scheduler;
}

void Simulation::Initialize(int argc, const char** argv,
                               const std::function<void(Param*)>& set_param) {
  id_ = counter_++;
  Activate();
  InitializeUniqueName(ExtractSimulationName(argv[0]));
  InitializeRuntimeParams(argc, argv, set_param);
  InitializeOutputDir();
  InitializeMembers();
}

void Simulation::InitializeMembers() {
  random_.resize(omp_get_max_threads());
  for (uint64_t i = 0; i < random_.size(); i++) {
    random_[i] = new Random();
  }
  exec_ctxt_.resize(omp_get_max_threads());
  for (uint64_t i = 0; i < exec_ctxt_.size(); i++) {
    exec_ctxt_[i] = new InPlaceExecutionContext();
  }
  rm_ = new ResourceManager();
  grid_ = new Grid();
  scheduler_ = new Scheduler();
}

void Simulation::InitializeRuntimeParams(int argc, const char** argv,
                                            const std::function<void(Param*)>& set_param) {
  param_ = new Param();

  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ",
            Version::String());

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
  set_param(param_);
}

void Simulation::InitializeUniqueName(const std::string& simulation_name) {
  name_ = simulation_name;
  std::stringstream stream;
  stream << name_;
  if (id_ > 0) {
    stream << id_;
  }
  unique_name_ = stream.str();
}

std::string Simulation::ExtractSimulationName(const char* path) {
  std::string s(path);
  auto pos = s.find_last_of("/");
  if (pos == std::string::npos) {
    return s;
  } else {
    return s.substr(pos + 1, s.length() - 1);
  }
}

void Simulation::InitializeOutputDir() {
  if (unique_name_ == "") {
    output_dir_ = param_->output_dir_;
  } else {
    output_dir_ = Concat(param_->output_dir_, "/", unique_name_);
  }
  if (system(Concat("mkdir -p ", output_dir_).c_str())) {
    Log::Fatal("Simulation", "Failed to make output directory ", output_dir_);
  }
}

}  // namespace bdm
