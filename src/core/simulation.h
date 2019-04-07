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
#ifndef CORE_SIMULATION_H_
#define CORE_SIMULATION_H_

#include <string>
#include <vector>
#include "core/util/random.h"
#include "core/util/root.h"

namespace bdm {

// forward declarations
template <typename>
class ResourceManager;
template <typename>
class Grid;
template <typename>
class Scheduler;
struct Param;
template <typename>
class InPlaceExecutionContext;

struct Soa;
template <typename TBackend = Soa>
struct CompileTimeParam;
class SimulationTest;
class CatalystAdaptorTest;

/// This is the central BioDynaMo object. It containes pointers to e.g. the
/// ResourceManager, the scheduler, parameters, ... \n
/// It is possible to create multiple simulations, but only one can be active at
/// the same time. Creating a new simulation object automatically activates it.
/// Implementation for `Simulation` can be found in file:
/// `simulation_implementation.h`. It must be separate to avoid circular
/// dependencies. It can't be defined in a source file, because it is templated.
template <typename TCTParam = CompileTimeParam<>>
struct Simulation {
  using ResourceManager_t = ResourceManager<TCTParam>;  // NOLINT
  using Param_t = typename TCTParam::Param;

  /// This function returns the currently active Simulation simulation.
  static Simulation<TCTParam>* GetActive();

  explicit Simulation(TRootIOCtor* p);
  /// Constructor that takes the arguments from `main` to parse command line
  /// arguments. The simulation name is extracted from the executable name.
  /// Creation of a new simulation automatically activates it.
  Simulation(int argc, const char** argv);

  /// Alternative constructor, if the arguments from function `main` are not
  /// available, or if a different simulation name should be chosen. \n
  /// Command line arguments are not parsed!\n
  /// Creation of a new simulation automatically activates it.
  explicit Simulation(const std::string& simulation_name);

  template <typename TSetParamLambda>
  Simulation(int argc, const char** argv, const TSetParamLambda& set_param);

  template <typename TSetParamLambda>
  Simulation(const std::string& simulation_name,
             const TSetParamLambda& set_param);

  ~Simulation();

  /// Copies / moves values from a restored simulation into this object.
  /// Thus, pointers to `rm_`, `param_`, ... are not invalidated.
  void Restore(Simulation&& restored);

  /// Activates this simulation.
  void Activate();

  ResourceManager<TCTParam>* GetResourceManager();

  const Param_t* GetParam() const;

  Grid<Simulation>* GetGrid();

  Scheduler<Simulation>* GetScheduler();

  /// Returns a thread local random number generator
  Random* GetRandom();

  /// Returns a thread local execution context.
  InPlaceExecutionContext<TCTParam>* GetExecutionContext();

  /// Returns the main execution context.
  InPlaceExecutionContext<TCTParam>* GetMainExecCtxt();

  /// Returns all thread local execution contexts.
  std::vector<InPlaceExecutionContext<TCTParam>*>& GetAllExecCtxts();

  /// @see `unique_name_`
  const std::string& GetUniqueName() const;

  /// Returns the output directory for this specific simulation
  const std::string& GetOutputDir() const;

  /// Replaces the scheduler for this simulation.
  /// Existing scheduler will be deleted! Therefore, pointers to the old
  /// scheduler (obtained with `GetScheduler()`) will be invalidated. \n
  /// Simulation will take ownership of the passed pointer
  void ReplaceScheduler(Scheduler<Simulation>*);

 private:
  /// Currently active simulation
  static Simulation<TCTParam>* active_;
  /// Number of simulations in this process
  static std::atomic<uint64_t> counter_;

  /// random number generator for each thread
  std::vector<Random*> random_;

  /// Execution Context for each thread
  std::vector<InPlaceExecutionContext<TCTParam>*> exec_ctxt_;  //!

  ResourceManager<TCTParam>* rm_ = nullptr;
  Param_t* param_ = nullptr;
  std::string name_;
  Grid<Simulation>* grid_ = nullptr;            //!
  Scheduler<Simulation>* scheduler_ = nullptr;  //!
  /// This id is unique for each simulation within the same process
  uint64_t id_ = 0;  //!
  /// cached value where `id_` is appended to `name_` if `id_` is
  /// not zero.\n
  /// e.g. `name_ = "my-sim"` and `id_ = 0` -> "my-sim"\n
  /// e.g. `name_ = "my-sim"` and `id_ = 4` -> "my-sim4"
  std::string unique_name_;  //!
  /// cached value where `unique_name_` is appended to `Param::output_dir_`
  std::string output_dir_;  //!

  /// Initialize Simulation
  template <typename TSetParamLambda>
  void Initialize(int argc, const char** argv,
                  const TSetParamLambda& set_param);

  /// Initialize data members that have a dependency on Simulation
  template <typename TResourceManager = ResourceManager<TCTParam>,
            typename TGrid = Grid<Simulation>,
            typename TScheduler = Scheduler<Simulation>>
  void InitializeMembers();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  std::string ExtractSimulationName(const char* path);

  /// This function parses command line parameters and the configuration file.
  /// @param argc argument count from main function
  /// @param argv argument vector from main function
  template <typename TSetParamLambda>
  void InitializeRuntimeParams(int argc, const char** argv,
                               const TSetParamLambda& set_param);

  /// This function initialzes `unique_name_`
  void InitializeUniqueName(const std::string& simulation_name);

  /// Initializes `output_dir_` and creates dir if it does not exist.
  void InitializeOutputDir();

  friend SimulationTest;
  friend CatalystAdaptorTest;

  BDM_CLASS_DEF_NV(Simulation, 1);
};

}  // namespace bdm

#endif  // CORE_SIMULATION_H_
