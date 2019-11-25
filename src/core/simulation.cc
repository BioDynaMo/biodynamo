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

#include "core/simulation.h"

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
#include "core/util/io.h"
#include "core/util/log.h"
#include "core/util/string.h"
#include "core/util/thread_info.h"
#include "core/visualization/root_adaptor.h"
#include "version.h"

namespace bdm {

/// Implementation for `Simulation`:
/// It must be separate to avoid circular dependencies.
/// It can't be defined in a source file, because it is templated.

std::atomic<uint64_t> Simulation::counter_;

Simulation* Simulation::active_ = nullptr;

Simulation* Simulation::GetActive() { return active_; }

Simulation::Simulation(TRootIOCtor* p) {}

Simulation::Simulation(int argc, const char** argv,
                       const std::string& config_file)
    : Simulation(argc, argv, [](auto* param) {}, config_file) {}

Simulation::Simulation(const std::string& simulation_name,
                       const std::string& config_file)
    : Simulation(simulation_name, [](auto* param) {}, config_file) {}

Simulation::Simulation(CommandLineOptions* clo,
                       const std::string& config_file) {
  Initialize(clo, [](auto* param) {}, config_file);
}

Simulation::Simulation(CommandLineOptions* clo,
                       const std::function<void(Param*)>& set_param,
                       const std::string& config_file) {
  Initialize(clo, set_param, config_file);
}

Simulation::Simulation(int argc, const char** argv,
                       const std::function<void(Param*)>& set_param,
                       const std::string& config_file) {
  auto options = CommandLineOptions(argc, argv);
  Initialize(&options, set_param, config_file);
}

Simulation::Simulation(const std::string& simulation_name,
                       const std::function<void(Param*)>& set_param,
                       const std::string& config_file) {
  const char* argv[1] = {simulation_name.c_str()};
  auto options = CommandLineOptions(1, argv);
  Initialize(&options, set_param, config_file);
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
  if (param_ != nullptr && rm_ != nullptr && param_->debug_numa_) {
    std::cout << "ThreadInfo:\n" << *ThreadInfo::GetInstance() << std::endl;
    rm_->DebugNuma();
  }
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

void Simulation::Activate() { active_ = this; }

/// Returns the ResourceManager instance
ResourceManager* Simulation::GetResourceManager() { return rm_; }

void Simulation::SetResourceManager(ResourceManager* rm) {
  delete rm_;
  rm_ = rm;
}

/// Returns the simulation parameters
const Param* Simulation::GetParam() const { return param_; }

Grid* Simulation::GetGrid() { return grid_; }

Scheduler* Simulation::GetScheduler() { return scheduler_; }

void Simulation::Simulate(uint64_t steps) { scheduler_->Simulate(steps); }

/// Returns a random number generator (thread-specific)
Random* Simulation::GetRandom() { return random_[omp_get_thread_num()]; }

std::vector<Random*>& Simulation::GetAllRandom() { return random_; }

InPlaceExecutionContext* Simulation::GetExecutionContext() {
  return exec_ctxt_[omp_get_thread_num()];
}

std::vector<InPlaceExecutionContext*>& Simulation::GetAllExecCtxts() {
  return exec_ctxt_;
}

/// Returns the name of the simulation
const std::string& Simulation::GetUniqueName() const { return unique_name_; }

/// Returns the path to the simulation's output directory
const std::string& Simulation::GetOutputDir() const { return output_dir_; }

void Simulation::ReplaceScheduler(Scheduler* scheduler) {
  delete scheduler_;
  scheduler_ = scheduler;
}

void Simulation::Initialize(CommandLineOptions* clo,
                            const std::function<void(Param*)>& set_param,
                            const std::string& config_file) {
  id_ = counter_++;
  Activate();
  if (!clo) {
    Log::Fatal("Simulation::Initialize",
               "CommandLineOptions argument was a nullptr!");
  }
  InitializeUniqueName(clo->GetSimulationName());
  InitializeRuntimeParams(clo, set_param, config_file);
  InitializeOutputDir();
  InitializeMembers();
}

void Simulation::InitializeMembers() {
  if (param_->debug_numa_) {
    std::cout << "ThreadInfo:\n" << *ThreadInfo::GetInstance() << std::endl;
  }

  random_.resize(omp_get_max_threads());
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < random_.size(); i++) {
    random_[i] = new Random();
  }
  exec_ctxt_.resize(omp_get_max_threads());
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < exec_ctxt_.size(); i++) {
    exec_ctxt_[i] = new InPlaceExecutionContext();
  }
  rm_ = new ResourceManager();
  grid_ = new Grid();
  scheduler_ = new Scheduler();
}

void Simulation::InitializeRuntimeParams(
    CommandLineOptions* clo, const std::function<void(Param*)>& set_param,
    const std::string& ctor_config) {
  // Renew thread info just in case it has been initialised as static and a
  // simulation calls e.g. `omp_set_num_threads()` within main.
  ThreadInfo::GetInstance()->Renew();

  param_ = new Param();

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDM_CMAKE_DIR") == nullptr) {
    Log::Fatal("Simulation::InitializeRuntimeParams",
               "The BioDynaMo environment is not set up correctly. Please call "
               "$use_biodynamo and retry this command.");
  }

  LoadConfigFile(ctor_config, clo->Get<std::string>("config"));
  if (clo->Get<std::string>("backup") != "") {
    param_->backup_file_ = clo->Get<std::string>("backup");
  }
  if (clo->Get<std::string>("restore") != "") {
    param_->restore_file_ = clo->Get<std::string>("restore");
  }

  // Handle "cuda" and "opencl" arguments
  if (clo->Get<bool>("cuda")) {
    param_->use_gpu_ = true;
  }

  if (clo->Get<bool>("opencl")) {
    param_->use_gpu_ = true;
    param_->use_opencl_ = true;
  }

  set_param(param_);

  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ",
            Version::String());
}

void Simulation::LoadConfigFile(const std::string& ctor_config,
                                const std::string& cli_config) {
  constexpr auto kConfigFile = "bdm.toml";
  constexpr auto kConfigFileParentDir = "../bdm.toml";
  if (ctor_config != "") {
    if (FileExists(ctor_config)) {
      auto config = cpptoml::parse_file(ctor_config);
      param_->AssignFromConfig(config);
    } else {
      Log::Fatal("Simulation::InitializeRuntimeParams", "The config file ",
                 ctor_config,
                 " specified in the constructor of bdm::Simulation "
                 "could not be found.");
    }
  } else if (cli_config != "") {
    if (FileExists(cli_config)) {
      auto config = cpptoml::parse_file(cli_config);
      param_->AssignFromConfig(config);
    } else {
      Log::Fatal("Simulation::InitializeRuntimeParams", "The config file ",
                 cli_config,
                 " specified as command line argument "
                 "could not be found.");
    }
  } else if (FileExists(kConfigFile)) {
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
