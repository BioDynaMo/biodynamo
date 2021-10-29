// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/simulation.h"

#include <cpptoml/cpptoml.h>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "bdm_version.h"
#include "core/agent/agent_uid_generator.h"
#include "core/analysis/time_series.h"
#include "core/environment/environment.h"
#include "core/environment/kd_tree_environment.h"
#include "core/environment/octree_environment.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/gpu/gpu_helper.h"
#include "core/param/command_line_options.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/util/filesystem.h"
#include "core/util/io.h"
#include "core/util/log.h"
#include "core/util/string.h"
#include "core/util/thread_info.h"
#include "core/util/timing.h"
#include "core/visualization/root/adaptor.h"
#include "memory_usage.h"

#include <TEnv.h>
#include <TROOT.h>

namespace fs = std::experimental::filesystem;

namespace bdm {

/// Implementation for `Simulation`:
/// It must be separate to avoid circular dependencies.

std::atomic<uint64_t> Simulation::counter_;

Simulation* Simulation::active_ = nullptr;

Simulation* Simulation::GetActive() { return active_; }

Simulation::Simulation(TRootIOCtor* p) {}

Simulation::Simulation(int argc, const char** argv,
                       const std::vector<std::string>& config_files)
    : Simulation(
          argc, argv, [](auto* param) {}, config_files) {}

Simulation::Simulation(const std::string& simulation_name,
                       const std::vector<std::string>& config_files)
    : Simulation(
          simulation_name, [](auto* param) {}, config_files) {}

Simulation::Simulation(CommandLineOptions* clo,
                       const std::vector<std::string>& config_files) {
  Initialize(
      clo, [](auto* param) {}, config_files);
}

Simulation::Simulation(CommandLineOptions* clo,
                       const std::function<void(Param*)>& set_param,
                       const std::vector<std::string>& config_files) {
  Initialize(clo, set_param, config_files);
}

Simulation::Simulation(int argc, const char** argv,
                       const std::function<void(Param*)>& set_param,
                       const std::vector<std::string>& config_files) {
  auto options = CommandLineOptions(argc, argv);
  Initialize(&options, set_param, config_files);
}

Simulation::Simulation(const std::string& simulation_name,
                       const std::function<void(Param*)>& set_param,
                       const std::vector<std::string>& config_files) {
  const char* argv[1] = {simulation_name.c_str()};
  auto options = CommandLineOptions(1, argv);
  Initialize(&options, set_param, config_files);
}

void Simulation::Restore(Simulation&& restored) {
  // random_
  if (random_.size() != restored.random_.size()) {
    Log::Warning("Simulation", "The restore file (", param_->restore_file,
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

  *time_series_ = std::move(*restored.time_series_);

  // name_ and unique_name_
  InitializeUniqueName(restored.name_);
  InitializeOutputDir();
}

std::ostream& operator<<(std::ostream& os, Simulation& sim) {
  std::vector<std::string> dgrid_names;
  std::vector<int> dgrid_resolutions;
  std::vector<std::array<int32_t, 3>> dgrid_dimensions;
  std::vector<uint64_t> dgrid_voxels;

  sim.rm_->ForEachDiffusionGrid([&](auto* dgrid) {
    dgrid_names.push_back(dgrid->GetSubstanceName());
    dgrid_resolutions.push_back(dgrid->GetResolution());
    dgrid_dimensions.push_back(dgrid->GetGridSize());
    dgrid_voxels.push_back(dgrid->GetNumBoxes());
  });

  os << std::endl;

  os << "***********************************************" << std::endl;
  os << "***********************************************" << std::endl;
  os << "\033[1mSimulation Metadata:\033[0m" << std::endl;
  os << "***********************************************" << std::endl;
  os << std::endl;
  os << "\033[1mGeneral\033[0m" << std::endl;
  if (sim.command_line_parameter_str_ != "") {
    os << "Command\t\t\t\t: " << sim.command_line_parameter_str_ << std::endl;
  }
  os << "Simulation name\t\t\t: " << sim.GetUniqueName() << std::endl;
  os << "Total simulation runtime\t: " << (sim.dtor_ts_ - sim.ctor_ts_) << " ms"
     << std::endl;
  os << "Peak memory usage (MB)\t\t: " << (getPeakRSS() / 1048576.0)
     << std::endl;
  os << "Number of iterations executed\t: "
     << sim.scheduler_->GetSimulatedSteps() << std::endl;
  os << "Number of agents\t\t: " << sim.rm_->GetNumAgents() << std::endl;

  if (dgrid_names.size() != 0) {
    os << "Diffusion grids" << std::endl;
    for (size_t i = 0; i < dgrid_names.size(); ++i) {
      os << "  " << dgrid_names[i] << ":" << std::endl;
      auto& dim = dgrid_dimensions[i];
      os << "\t"
         << "Resolution\t\t: " << dgrid_resolutions[i] << std::endl;
      os << "\t"
         << "Size\t\t\t: " << dim[0] << " x " << dim[1] << " x " << dim[2]
         << std::endl
         << "\tVoxels\t\t\t: " << dgrid_voxels[i] << std::endl;
    }
  }

  os << "Output directory\t\t: " << sim.GetOutputDir() << std::endl;
  os << "  size\t\t\t\t: "
     << gSystem->GetFromPipe(
            Concat("du -sh ", sim.GetOutputDir(), " | cut -f1").c_str())
     << std::endl;
  os << "BioDynaMo version:\t\t: " << Version::String() << std::endl;
  os << std::endl;
  os << "***********************************************" << std::endl;
  os << *(sim.scheduler_->GetOpTimes()) << std::endl;
  os << "***********************************************" << std::endl;
  os << std::endl;
  os << "\033[1mThread Info\033[0m" << std::endl;
  os << *ThreadInfo::GetInstance();
  os << std::endl;
  os << "***********************************************" << std::endl;
  os << std::endl;
  os << *(sim.rm_);
  os << std::endl;
  os << "***********************************************" << std::endl;
  os << std::endl;
  os << "\033[1mParameters\033[0m" << std::endl;
  os << sim.param_->ToJsonString();
  os << std::endl;
  os << "***********************************************" << std::endl;
  os << "***********************************************" << std::endl;

  return os;
}

Simulation::~Simulation() {
  dtor_ts_ = bdm::Timing::Timestamp();

  if (param_ != nullptr && param_->statistics) {
    std::stringstream sstr;
    sstr << *this << std::endl;
    std::cout << sstr.str() << std::endl;
    // write to file
    std::ofstream ofs(Concat(output_dir_, "/metadata"));
    ofs << sstr.str() << std::endl;
  }

  if (mem_mgr_) {
    mem_mgr_->SetIgnoreDelete(true);
  }
  Simulation* tmp = nullptr;
  if (active_ != this) {
    tmp = active_;
  }
  active_ = this;

  delete rm_;
  delete environment_;
  delete scheduler_;
  if (agent_uid_generator_ != nullptr) {
    delete agent_uid_generator_;
  }
  delete param_;
  for (auto* r : random_) {
    delete r;
  }
  for (auto* ectxt : exec_ctxt_) {
    delete ectxt;
  }
  if (mem_mgr_) {
    delete mem_mgr_;
  }
  delete ocl_state_;
  if (time_series_) {
    delete time_series_;
  }
  active_ = tmp;
}

void Simulation::Activate() { active_ = this; }

/// Returns the ResourceManager instance
ResourceManager* Simulation::GetResourceManager() { return rm_; }

void Simulation::SetResourceManager(ResourceManager* rm) {
  if (rm != rm_) {
    delete rm_;
  }
  rm_ = rm;
}

/// Returns the simulation parameters
const Param* Simulation::GetParam() const { return param_; }

AgentUidGenerator* Simulation::GetAgentUidGenerator() {
  return agent_uid_generator_;
}

Environment* Simulation::GetGrid() { return environment_; }

Environment* Simulation::GetEnvironment() { return environment_; }

Scheduler* Simulation::GetScheduler() { return scheduler_; }

void Simulation::Simulate(uint64_t steps) { scheduler_->Simulate(steps); }

/// Returns a random number generator (thread-specific)
Random* Simulation::GetRandom() { return random_[omp_get_thread_num()]; }

std::vector<Random*>& Simulation::GetAllRandom() { return random_; }

ExecutionContext* Simulation::GetExecutionContext() {
  return exec_ctxt_[omp_get_thread_num()];
}

std::vector<ExecutionContext*>& Simulation::GetAllExecCtxts() {
  return exec_ctxt_;
}

OpenCLState* Simulation::GetOpenCLState() { return ocl_state_; }

/// Returns the name of the simulation
const std::string& Simulation::GetUniqueName() const { return unique_name_; }

/// Returns the path to the simulation's output directory
const std::string& Simulation::GetOutputDir() const { return output_dir_; }

experimental::TimeSeries* Simulation::GetTimeSeries() { return time_series_; }

void Simulation::ReplaceScheduler(Scheduler* scheduler) {
  delete scheduler_;
  scheduler_ = scheduler;
}

void Simulation::Initialize(CommandLineOptions* clo,
                            const std::function<void(Param*)>& set_param,
                            const std::vector<std::string>& config_files) {
  // Initialize a thread-safe ROOT instance
  TROOT(name_.c_str(), "BioDynaMo");
  ROOT::EnableThreadSafety();

  ctor_ts_ = bdm::Timing::Timestamp();
  id_ = counter_++;
  Activate();
  if (!clo) {
    Log::Fatal("Simulation::Initialize",
               "CommandLineOptions argument was a nullptr!");
  }
  InitializeUniqueName(clo->GetSimulationName());
  InitializeRuntimeParams(clo, set_param, config_files);
  InitializeOutputDir();
  InitializeMembers();
}

void Simulation::InitializeMembers() {
  if (param_->use_bdm_mem_mgr) {
    mem_mgr_ = new MemoryManager(param_->mem_mgr_aligned_pages_shift,
                                 param_->mem_mgr_growth_rate,
                                 param_->mem_mgr_max_mem_per_thread_factor);
  }
  agent_uid_generator_ = new AgentUidGenerator();
  if (param_->debug_numa) {
    std::cout << "ThreadInfo:\n" << *ThreadInfo::GetInstance() << std::endl;
  }

  random_.resize(omp_get_max_threads());
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < random_.size(); i++) {
    random_[i] = new Random();
    random_[i]->SetSeed(param_->random_seed * (i + 1));
  }
  exec_ctxt_.resize(omp_get_max_threads());
  auto map = std::make_shared<
      typename InPlaceExecutionContext::ThreadSafeAgentUidMap>();
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < exec_ctxt_.size(); i++) {
    exec_ctxt_[i] = new InPlaceExecutionContext(map);
  }
  rm_ = new ResourceManager();

  // Set the specified neighborhood search method
  if (param_->environment == "kd_tree") {
    environment_ = new KDTreeEnvironment();
  } else if (param_->environment == "octree") {
    environment_ = new OctreeEnvironment();
  } else if (param_->environment == "uniform_grid") {
    environment_ = new UniformGridEnvironment();
  } else {
    Log::Error("Simulation::Initialize", "No such neighboring method '",
               param_->environment, "'. Defaulting to 'uniform_grid'");
    environment_ = new UniformGridEnvironment();
  }
  scheduler_ = new Scheduler();
  time_series_ = new experimental::TimeSeries();
}

void Simulation::SetEnvironment(Environment* env) {
  if (environment_ != nullptr && env != environment_) {
    delete environment_;
  }
  environment_ = env;
}

void Simulation::SetAllExecCtxts(
    const std::vector<ExecutionContext*>& exec_ctxts) {
  if (exec_ctxts.size() != exec_ctxt_.size()) {
    Log::Error("Simulation::SetAllExecCtxts", "Size of exec_ctxts (",
               exec_ctxts.size(), ") does not match the expected size (",
               exec_ctxt_.size(), "). Operation aborted");
    return;
  }
  for (uint64_t i = 0; i < exec_ctxt_.size(); ++i) {
    auto* ctxt = exec_ctxt_[i];
    if (ctxt != nullptr && ctxt != exec_ctxts[i]) {
      delete ctxt;
    }
    exec_ctxt_[i] = exec_ctxts[i];
  }
}

void Simulation::InitializeRuntimeParams(
    CommandLineOptions* clo, const std::function<void(Param*)>& set_param,
    const std::vector<std::string>& ctor_config_files) {
  // Renew thread info just in case it has been initialised as static and a
  // simulation calls e.g. `omp_set_num_threads()` within main.
  ThreadInfo::GetInstance()->Renew();

  std::stringstream sstr;
  sstr << (*clo);
  command_line_parameter_str_ = sstr.str();

  param_ = new Param();

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDMSYS") == nullptr) {
    Log::Fatal(
        "Simulation::InitializeRuntimeParams",
        "The BioDynaMo environment is not set up correctly. Please execute "
        "'source <path-to-bdm-installation>/bin/thisbdm.sh' and retry this "
        "command.");
  }

  static bool read_env = false;
  if (!read_env) {
    // Read, only once, bdm.rootrc to set BioDynaMo-related settings for ROOT
    std::stringstream os;
    os << std::getenv("BDMSYS") << "/etc/bdm.rootrc";
    gEnv->ReadFile(os.str().c_str(), kEnvUser);
    read_env = true;
  }

  LoadConfigFiles(ctor_config_files,
                  clo->Get<std::vector<std::string>>("config"));
  auto inline_configs = clo->Get<std::vector<std::string>>("inline-config");
  if (inline_configs.size()) {
    for (auto& inline_config : inline_configs) {
      param_->MergeJsonPatch(inline_config);
    }
  }

  if (clo->Get<std::string>("backup") != "") {
    param_->backup_file = clo->Get<std::string>("backup");
  }
  if (clo->Get<std::string>("restore") != "") {
    param_->restore_file = clo->Get<std::string>("restore");
  }

  // Handle "cuda" and "opencl" arguments
  if (clo->Get<bool>("cuda")) {
    param_->compute_target = "cuda";
  }

  if (clo->Get<bool>("opencl")) {
    param_->compute_target = "opencl";
  }

  ocl_state_ = new OpenCLState();

  set_param(param_);

  if (!is_gpu_environment_initialized_ && param_->compute_target != "cpu") {
    GpuHelper::GetInstance()->InitializeGPUEnvironment();
    is_gpu_environment_initialized_ = true;
  }

  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ",
            Version::String());
}

void Simulation::LoadConfigFiles(const std::vector<std::string>& ctor_configs,
                                 const std::vector<std::string>& cli_configs) {
  constexpr auto kTomlConfigFile = "bdm.toml";
  constexpr auto kJsonConfigFile = "bdm.json";
  constexpr auto kTomlConfigFileParentDir = "../bdm.toml";
  constexpr auto kJsonConfigFileParentDir = "../bdm.json";
  // find config file
  std::vector<std::string> configs = {};
  if (ctor_configs.size()) {
    for (auto& ctor_config : ctor_configs) {
      if (FileExists(ctor_config)) {
        configs.push_back(ctor_config);
      } else {
        Log::Fatal("Simulation::LoadConfigFiles", "The config file ",
                   ctor_config,
                   " specified in the constructor of bdm::Simulation "
                   "could not be found.");
      }
    }
  }
  if (cli_configs.size()) {
    for (auto& cli_config : cli_configs) {
      if (FileExists(cli_config)) {
        configs.push_back(cli_config);
      } else {
        Log::Fatal("Simulation::LoadConfigFiles", "The config file ",
                   cli_config,
                   " specified as command line argument "
                   "could not be found.");
      }
    }
  }

  // no config file specified in ctor or cli -> look for default
  if (!configs.size()) {
    if (FileExists(kTomlConfigFile)) {
      configs.push_back(kTomlConfigFile);
    } else if (FileExists(kTomlConfigFileParentDir)) {
      configs.push_back(kTomlConfigFileParentDir);
    } else if (FileExists(kJsonConfigFile)) {
      configs.push_back(kJsonConfigFile);
    } else if (FileExists(kJsonConfigFileParentDir)) {
      configs.push_back(kJsonConfigFileParentDir);
    }
  }

  // load config files
  if (configs.size()) {
    for (auto& config : configs) {
      if (EndsWith(config, ".toml")) {
        auto toml = cpptoml::parse_file(config);
        param_->AssignFromConfig(toml);
      } else if (EndsWith(config, ".json")) {
        std::ifstream ifs(config);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        param_->MergeJsonPatch(buffer.str());
      }
      Log::Info("Simulation::LoadConfigFiles",
                "Processed config file: ", config);
    }
  } else {
    Log::Info("Simulation::LoadConfigFiles", "Default config file ",
              kTomlConfigFile, " or ", kJsonConfigFile,
              " not found in `.` or `..` directory. No other config file was "
              "specified as command line parameter or passed to the "
              "constructor of bdm::Simulation.");
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
    output_dir_ = param_->output_dir;
  } else {
    output_dir_ = Concat(param_->output_dir, "/", unique_name_);
  }
  // If we do not remove the output directory, we add a timestamp to the
  // output directory to avoid overriding previous results.
  if (!param_->remove_output_dir_contents) {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "/%Y-%m-%d-%H:%M:%S", timeinfo);
    output_dir_ += buffer;
  }

  if (system(Concat("mkdir -p ", output_dir_).c_str())) {
    Log::Fatal("Simulation::InitializeOutputDir",
               "Failed to make output directory ", output_dir_);
  }
  if (!fs::is_empty(output_dir_)) {
    if (param_->remove_output_dir_contents) {
      RemoveDirectoryContents(output_dir_);
    } else {
      // We throw a fatal because it will override previous results from a
      // possibly expensive simulation. This should not happen unintentionally.
      Log::Fatal(
          "Simulation::InitializeOutputDir", "Output dir (", output_dir_,
          ") is not empty. Previous result files would be overriden. Abort."
          "Please set Param::remove_output_dir_contents to true to remove files"
          " automatically or clear the output directory by hand.");
    }
  }
}

}  // namespace bdm
