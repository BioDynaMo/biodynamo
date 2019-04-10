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

#include <functional>
#include <string>
#include <vector>
#include "core/util/random.h"
#include "core/util/root.h"

namespace bdm {

// forward declarations
class ResourceManager;
class Grid;
class Scheduler;
struct Param;
class InPlaceExecutionContext;

class SimulationTest;
class CatalystAdaptorTest;

/// This is the central BioDynaMo object. It containes pointers to e.g. the
/// ResourceManager, the scheduler, parameters, ... \n
/// It is possible to create multiple simulations, but only one can be active at
/// the same time. Creating a new simulation object automatically activates it.
struct Simulation {
  /// This function returns the currently active Simulation simulation.
  static Simulation* GetActive();

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

  Simulation(int argc, const char** argv,
             const std::function<void(Param*)>& set_param);

  Simulation(const std::string& simulation_name,
             const std::function<void(Param*)>& set_param);

  ~Simulation();

  /// Copies / moves values from a restored simulation into this object.
  /// Thus, pointers to `rm_`, `param_`, ... are not invalidated.
  void Restore(Simulation&& restored);

  /// Activates this simulation.
  void Activate();

  ResourceManager* GetResourceManager();

  const Param* GetParam() const;

  Grid* GetGrid();

  Scheduler* GetScheduler();

  /// Returns a thread local random number generator.
  Random* GetRandom();

  /// Returns all thread local random number generator.
  std::vector<Random*>& GetAllRandom();

  /// Returns a thread local execution context.
  InPlaceExecutionContext* GetExecutionContext();

  /// Returns all thread local execution contexts.
  std::vector<InPlaceExecutionContext*>& GetAllExecCtxts();

  /// @see `unique_name_`
  const std::string& GetUniqueName() const;

  /// Returns the output directory for this specific simulation
  const std::string& GetOutputDir() const;

  /// Replaces the scheduler for this simulation.
  /// Existing scheduler will be deleted! Therefore, pointers to the old
  /// scheduler (obtained with `GetScheduler()`) will be invalidated. \n
  /// Simulation will take ownership of the passed pointer
  void ReplaceScheduler(Scheduler*);

 private:
  /// Currently active simulation
  static Simulation* active_;
  /// Number of simulations in this process
  static std::atomic<uint64_t> counter_;

  /// random number generator for each thread
  std::vector<Random*> random_;

  /// Execution Context for each thread
  std::vector<InPlaceExecutionContext*> exec_ctxt_;  //!

  ResourceManager* rm_ = nullptr;
  Param* param_ = nullptr;
  std::string name_;
  Grid* grid_ = nullptr;            //!
  Scheduler* scheduler_ = nullptr;  //!
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
  void Initialize(int argc, const char** argv,
                  const std::function<void(Param*)>& set_param);

  /// Initialize data members that have a dependency on Simulation
  void InitializeMembers();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  std::string ExtractSimulationName(const char* path);

  /// This function parses command line parameters and the configuration file.
  /// @param argc argument count from main function
  /// @param argv argument vector from main function
  void InitializeRuntimeParams(int argc, const char** argv,
                               const std::function<void(Param*)>& set_param);

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
