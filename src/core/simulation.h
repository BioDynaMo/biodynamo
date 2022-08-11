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
#ifndef CORE_SIMULATION_H_
#define CORE_SIMULATION_H_

#include <functional>
#include <string>
#include <vector>

#include "core/agent/agent_uid.h"
#include "core/gpu/opencl_state.h"
#include "core/memory/memory_manager.h"
#include "core/util/random.h"
#include "core/util/root.h"

namespace bdm {

// forward declarations
class ResourceManager;
class Environment;
class Grid;
class Scheduler;
struct Param;
class ExecutionContext;
class CommandLineOptions;
class AgentUidGenerator;
class SimulationSpace;

class SimulationTest;
class ParaviewAdaptorTest;

namespace experimental {
class TimeSeries;
class Distributor;
}  // namespace experimental

/// This is the central BioDynaMo object. It containes pointers to e.g. the
/// ResourceManager, the scheduler, parameters, ... \n
/// It is possible to create multiple simulations, but only one can be active at
/// the same time. Creating a new agent automatically activates it.
class Simulation {
 public:
  /// This function returns the currently active Simulation simulation.
  static Simulation* GetActive();

  explicit Simulation(TRootIOCtor* p);
  /// Constructor that takes the arguments from `main` to parse command line
  /// arguments. The simulation name is extracted from the executable name.
  /// Creation of a new simulation automatically activates it.
  Simulation(int argc, const char** argv,
             const std::vector<std::string>& config_files = {});

  explicit Simulation(CommandLineOptions* clo,
                      const std::vector<std::string>& config_files = {});

  /// Alternative constructor, if the arguments from function `main` are not
  /// available, or if a different simulation name should be chosen. \n
  /// Command line arguments are not parsed!\n
  /// Creation of a new simulation automatically activates it.
  /// \param config_file Use a different config file than the default bdm.toml
  ///        or bdm.json
  explicit Simulation(const std::string& simulation_name,
                      const std::vector<std::string>& config_files = {});

  Simulation(int argc, const char** argv,
             const std::function<void(Param*)>& set_param,
             const std::vector<std::string>& config_files = {});

  Simulation(CommandLineOptions* clo,
             const std::function<void(Param*)>& set_param,
             const std::vector<std::string>& config_files = {});

  Simulation(const std::string& simulation_name,
             const std::function<void(Param*)>& set_param,
             const std::vector<std::string>& config_files = {});

  ~Simulation();

  /// Copies / moves values from a restored simulation into this object.
  /// Thus, pointers to `rm_`, `param_`, ... are not invalidated.
  void Restore(Simulation&& restored);

  /// Activates this simulation.
  void Activate();

  ResourceManager* GetResourceManager();

  void SetResourceManager(ResourceManager* rm);

  const Param* GetParam() const;

  AgentUidGenerator* GetAgentUidGenerator();

  Environment* GetEnvironment();

  /// Set a specific environment for the simulation. *env must point to an
  /// object instance of a subclass of Environment.
  void SetEnvironment(Environment* env);

  Scheduler* GetScheduler();

  void Simulate(uint64_t steps);

  /// Returns a thread local random number generator.
  Random* GetRandom();

  /// Returns all thread local random number generator.
  std::vector<Random*>& GetAllRandom();

  /// Returns a thread local execution context.
  ExecutionContext* GetExecutionContext();

  /// Returns all thread local execution contexts.
  std::vector<ExecutionContext*>& GetAllExecCtxts();

  void SetAllExecCtxts(const std::vector<ExecutionContext*>& exec_ctxts);

  MemoryManager* GetMemoryManager() { return mem_mgr_; }

  /// Return helper class for OpenCL environment
  OpenCLState* GetOpenCLState();

  /// @see `unique_name_`
  const std::string& GetUniqueName() const;

  /// Returns the output directory for this specific simulation
  const std::string& GetOutputDir() const;

  experimental::TimeSeries* GetTimeSeries();

  SimulationSpace* GetSimulationSpace();

  experimental::Distributor* GetDistributor();

  /// Replaces the scheduler for this simulation.
  /// Existing scheduler will be deleted! Therefore, pointers to the old
  /// scheduler (obtained with `GetScheduler()`) will be invalidated. \n
  /// Simulation will take ownership of the passed pointer
  void ReplaceScheduler(Scheduler* scheduler);

 private:
  /// Currently active simulation
  static Simulation* active_;
  /// Number of simulations in this process
  static std::atomic<uint64_t> counter_;

  /// random number generator for each thread
  std::vector<Random*> random_;

  /// Execution Context for each thread
  std::vector<ExecutionContext*> exec_ctxt_;  //!

  ResourceManager* rm_ = nullptr;
  Param* param_ = nullptr;
  AgentUidGenerator* agent_uid_generator_ = nullptr;  //!
  std::string name_;
  Environment* environment_ = nullptr;  //!
  Scheduler* scheduler_ = nullptr;      //!
  OpenCLState* ocl_state_ = nullptr;    //!
  bool is_gpu_environment_initialized_ = false;
  /// This id is unique for each simulation within the same process
  uint64_t id_ = 0;  //!
  /// cached value where `id_` is appended to `name_` if `id_` is
  /// not zero.\n
  /// e.g. `name_ = "my-sim"` and `id_ = 0` -> "my-sim"\n
  /// e.g. `name_ = "my-sim"` and `id_ = 4` -> "my-sim4"
  std::string unique_name_;  //!
  /// cached value where `unique_name_` is appended to `Param::output_dir`
  std::string output_dir_;  //!
  /// Stores command line arguments if (argc,argv) or CommandLineOptions
  /// are passed to the constructor.\n
  std::string command_line_parameter_str_;  //!
  /// BioDynaMo memory manager. If nullptr, default allocator will be used.
  MemoryManager* mem_mgr_ = nullptr;  //!
  /// Timestep when constructor was called
  int64_t ctor_ts_ = 0;  //!
  /// Timestep when destructor was called
  int64_t dtor_ts_ = 0;  //!
  /// Collects time series information during the simulation
  experimental::TimeSeries* time_series_ = nullptr;
  /// object responsible to distributed the simulation among multiple MPI ranks
  experimental::Distributor* distributor_ = nullptr;

  /// Describes the simulation space
  SimulationSpace* space_;  //!

  /// Initialize Simulation
  void Initialize(CommandLineOptions* clo,
                  const std::function<void(Param*)>& set_param,
                  const std::vector<std::string>& config_files);

  /// Initialize data members that have a dependency on Simulation
  void InitializeMembers();

  /// This function parses command line parameters and the configuration file.
  void InitializeRuntimeParams(CommandLineOptions* clo,
                               const std::function<void(Param*)>& set_param,
                               const std::vector<std::string>& ctor_config);

  void LoadConfigFiles(const std::vector<std::string>& ctor_configs,
                       const std::vector<std::string>& cli_configs);

  /// This function initialzes `unique_name_`
  void InitializeUniqueName(const std::string& simulation_name);

  /// Initializes `output_dir_` and creates dir if it does not exist.
  void InitializeOutputDir();

  friend SimulationTest;
  friend ParaviewAdaptorTest;
  friend class DiffusionTest_CopyOldData_Test;
  friend std::ostream& operator<<(std::ostream& os, Simulation& sim);

  BDM_CLASS_DEF_NV(Simulation, 1);
};

}  // namespace bdm

#endif  // CORE_SIMULATION_H_
