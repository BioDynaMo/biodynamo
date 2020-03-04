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

#ifndef CORE_PARAM_PARAM_H_
#define CORE_PARAM_PARAM_H_

#include <cinttypes>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/param/module_param.h"
#include "core/util/root.h"
#include "core/util/type.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

class Simulation;

struct Param {
  static void RegisterModuleParam(ModuleParam* param);

  Param();

  ~Param();

  void Restore(Param&& other);

  template <typename TModuleParam>
  const TModuleParam* GetModuleParam() const {
    assert(modules_.find(TModuleParam::kUid) != modules_.end() &&
           "Couldn't find the requested module parameter.");
    return bdm_static_cast<const TModuleParam*>(
        modules_.at(TModuleParam::kUid));
  }

  template <typename TModuleParam>
  TModuleParam* GetModuleParam() {
    assert(modules_.find(TModuleParam::kUid) != modules_.end() &&
           "Couldn't find the requested module parameter.");
    return bdm_static_cast<TModuleParam*>(modules_.at(TModuleParam::kUid));
  }

  // simulation values ---------------------------------------------------------

  /// Variable which specifies method using for solving differential equation
  /// {"Euler", "RK4"}.
  enum NumericalODESolver { kEuler = 1, kRK4 = 2 };
  NumericalODESolver numerical_ode_solver_ = NumericalODESolver::kEuler;

  /// Ouput Directory name used to store visualization and other files.\n
  /// Path is relative to working directory.\n
  /// Default value: `"output"`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     ouput_dir = "output"
  std::string output_dir_ = "output";

  /// Backup file name for full simulation backups\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no backups will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_file = <path>/<filename>.root
  /// Command line argument: `-b, --backup`
  std::string backup_file_ = "";

  /// File name to restore simulation from\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no restore will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     restore_file = <path>/<filename>.root
  /// Command line argument: `-r, --restore`
  std::string restore_file_ = "";

  /// Specifies the interval (in seconds) in which backups will be performed.\n
  /// Default Value: `1800` (every half an hour)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_interval = 1800  # backup every half an hour
  uint32_t backup_interval_ = 1800;

  /// Time between two simulation steps, in hours.
  /// Default value: `0.01`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     time_step = 0.0125
  double simulation_time_step_ = 0.01;

  /// Maximum jump that a point mass can do in one time step. Useful to
  /// stabilize the simulation\n
  /// Default value: `3.0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_displacement = 3.0
  double simulation_max_displacement_ = 3.0;

  /// Calculate mechanical interactions between simulation objects.\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     run_mechanical_interactions = true
  bool run_mechanical_interactions_ = true;

  /// Enforce an artificial cubic bounds around the simulation space.
  /// Simulation objects cannot move outside this cube. Dimensions of this cube
  /// are determined by parameter `lbound` and `rbound`.\n
  /// Default value: `false` (simulation space is "infinite")\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     bound_space = false
  bool bound_space_ = false;

  /// Minimum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space_`).\n
  /// Default value: `0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     min_bound = 0
  double min_bound_ = 0;

  /// Maximum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space_`).\n
  /// Default value: `100`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_bound = 100
  double max_bound_ = 100;

  /// Allow substances to leak out of the simulation space. In this way
  /// the substance concentration will not be blocked by an artificial border\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     leaking_edges = true
  bool leaking_edges_ = true;

  /// Calculate the diffusion gradient for each substance.\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     calculate_gradients = true
  bool calculate_gradients_ = true;

  /// List of thread-safety mechanisms \n
  /// `kNone`: \n
  /// `kUserSpecified`: The user has to define all simulation object that must
  /// not be processed in parallel. \see `SimObject::CriticalRegion`.\n
  /// `kAutomatic`: The simulation automatically locks all simulation objects
  /// of the microenvironment.
  enum ThreadSafetyMechanism { kNone = 0, kUserSpecified, kAutomatic };
  /// Select the thread-safety mechanism.\n
  /// Possible values are: none, user-specified, automatic.\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     thread_safety_mechanism_ = "none"
  ThreadSafetyMechanism thread_safety_mechanism_ =
      ThreadSafetyMechanism::kUserSpecified;

  // visualization values ------------------------------------------------------

  /// Name of the visualization engine to use for visualizaing BioDynaMo
  /// simulations\n
  /// Default value: `"paraview"`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     adaptor = <name_of_adaptor>
  std::string visualization_engine_ = "paraview";

  /// Use ParaView Catalyst for live visualization.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     live = false
  bool live_visualization_ = false;

  /// Write data to file for post-simulation visualization
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export = false
  bool export_visualization_ = false;

  /// Use ROOT for enable visualization.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     root = false
  bool root_visualization_ = false;

  /// If `export_visualization_` is set to true, this parameter specifies
  /// how often it should be exported. 1 = every timestep, 10: every 10
  /// time steps.\n
  /// Default value: `1`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export_interval = 1
  uint32_t visualization_export_interval_ = 1;

  /// If `export_visualization_` is set to true, this parameter specifies
  /// if the ParaView pvsm file will be generated!\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export_generate_pvsm = true
  bool visualization_export_generate_pvsm_ = true;

  /// Specifies which simulation objects should be visualized. \n
  /// Every simulation object defines the minimum set of data members which
  /// are required to visualize it. (e.g. Cell: `position_` and `diameter_`).\n
  /// With this parameter it is also possible to extend the number of data
  /// members that are sent to the visualization engine.
  /// Default value: empty (no simulation object will be visualized)\n
  /// NB: This data member is not backed up, due to a ROOT error.
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on live or export
  ///     export = true
  ///
  ///       [[visualize_sim_object]]
  ///       name = "Cell"
  ///       # the following entry is optional
  ///       additional_data_members = [ "density_" ]
  ///
  ///       # The former block can be repeated for further simulation objects
  ///       [[visualize_sim_object]]
  ///       name = "Neurite"
  std::unordered_map<std::string, std::set<std::string>>
      visualize_sim_objects_;  //!

  struct VisualizeDiffusion {
    std::string name_;
    bool concentration_ = true;
    bool gradient_ = false;
  };

  /// Spceifies for which substances extracellular diffusion should be
  /// visualized.\n
  /// Default value: empty (no diffusion will be visualized)\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on live or export
  ///     export = true
  ///
  ///       [[visualize_diffusion]]
  ///       # Name of the substance
  ///       name = "Na"
  ///       # the following two entries are optional
  ///       #   default value for concentration is true
  ///       concentration = true
  ///       #   default value for gradient is false
  ///       gradient = false
  ///
  ///       # The former block can be repeated for further substances
  ///       [[visualize_diffusion]]
  ///       name = "K"
  ///       # default values: concentration = true and gradient = false
  std::vector<VisualizeDiffusion> visualize_diffusion_;

  // performance values --------------------------------------------------------

  /// Batch size used by the `Scheduler` to iterate over simulation objects\n
  /// Default value: `1000`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     scheduling_batch_size = 1000
  uint64_t scheduling_batch_size_ = 1000;

  /// Calculation of the displacement (mechanical interaction) is an
  /// expensive operation. If simulation objects do not move or grow,
  /// displacement calculation is ommited if detect_static_sim_objects is turned
  /// on. However, the detection mechanism introduces an overhead. For dynamic
  /// simulations where sim objects move and grow, the overhead outweighs the
  /// benefits.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     detect_static_sim_objects = false
  bool detect_static_sim_objects_ = false;

  /// Neighbors of a simulation object can be cached so to avoid consecutive
  /// searches. This of course only makes sense if there is more than one
  /// `ForEachNeighbor*` operation.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     cache_neighbors = false
  bool cache_neighbors_ = false;

  /// If the utilization in the SoUidMap inside ResourceManager falls below
  /// this watermark, defragmentation will be turned on.\n
  /// Default value: `0.5`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     souid_defragmentation_low_watermark = 0.5
  double souid_defragmentation_low_watermark_ = 0.5;

  /// If the utilization in the SoUidMap inside ResourceManager rises above
  /// this watermark, defragmentation will be turned off.\n
  /// Default value: `0.9`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     souid_defragmentation_high_watermark = 0.9
  double souid_defragmentation_high_watermark_ = 0.9;

  /// Use the BioDynaMo memory manager.
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     use_bdm_mem_mgr = true
  bool use_bdm_mem_mgr_ = true;

  /// The BioDynaMo memory manager allocates N page aligned memory blocks.
  /// The bigger N, the lower the memory overhead due to metadata storage
  /// if a lot of memory is used.\n
  /// N must be a number of two.\n
  /// Therefore, this parameter specifies the shift for N. `N = 2 ^ shift`\n
  /// Default value: `8` `-> N = 256`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_aligned_pages_shift = 8
  uint64_t mem_mgr_aligned_pages_shift_ = 8;

  /// The BioDynaMo memory manager allocates memory in increasing sizes using
  /// a geometric series. This parameter specifies the growth rate.
  /// Default value: `2.0`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_growth_rate = 1.1
  double mem_mgr_growth_rate_ = 1.1;

  /// The BioDynaMo memory manager can migrate memory between thread pools
  /// to avoid memory leaks.\n
  /// This parameter specifies the maximum memory size in bytes before
  /// migration happens.\n
  /// This value must be bigger than `PAGE_SIZE * 2 ^ mem_mgr_growth_rate_`\n
  /// Default value: `10485760` (10 MB)\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_max_mem_per_thread = 10485760
  uint64_t mem_mgr_max_mem_per_thread_ = 1024 * 1024 * 10;

  /// This parameter is used inside `ResourceManager::SortAndBalanceNumaNodes`.
  /// If it is set to true, the function will reuse existing memory to rebalance
  /// simulation objects to NUMA nodes. (A small amount of additional memory
  /// is still required.)\n
  /// If this parameter is set to false, the balancing function will first
  /// create new objects and delete the old ones in a second step. In the worst
  /// case this will double the required memory for simulation objects for.
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     minimize_memory_while_rebalancing = true
  bool minimize_memory_while_rebalancing_ = true;

  // development values --------------------------------------------------------
  /// Statistics of profiling data; keeps track of the execution time of each
  /// operation at every timestep.\n
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     statistics = false
  bool statistics_ = false;

  /// Output debugging info related to running on NUMA architecture.\n
  /// \see `ThreadInfo`, `ResourceManager::DebugNuma`
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     debug_numa = false
  bool debug_numa_ = false;

  /// Use the python script (simple_pipeline.py) to do Live Visualization with
  /// ParaView. If false, we use the C++ pipeline
  /// Default value: `false`\n
  /// TOML config file:
  ///     [development]
  ///     python_paraview_pipeline_ = false
  bool python_paraview_pipeline_ = false;

  /// Display the current simulation step in the terminal output
  /// Default value: `true`\n
  /// TOML config file:
  ///     [development]
  ///     show_simulation_step = true
  bool show_simulation_step_ = true;

  /// Sets the frequency at which the current simulation step is displayed.
  /// Display every `simulation_step_freq_` steps.
  /// Default value: `10`\n
  /// TOML config file:
  ///     [development]
  ///     simulation_step_freq = false
  uint32_t simulation_step_freq_ = 10;

  // ---------------------------------------------------------------------------
  // experimental group

  /// Run the simulation partially on the GPU for improved performance.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [experimental]
  ///     use_gpu = false
  bool use_gpu_ = false;

  /// When both CUDA and OpenCL are available on a machine, the preference to
  /// OpenCL can be set with this flag, as per default CUDA is used.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [experimental]
  ///     use_opencl = false
  bool use_opencl_ = false;

  /// Compile OpenCL kernels with debugging symbols, for debugging on CPU
  /// targets with GNU gdb.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [experimental]
  ///     opencl_debug_ = false
  bool opencl_debug_ = false;

  /// Set the index of the preferred GPU you wish to use.
  /// Default value: `0`\n
  /// TOML config file:
  ///     [experimental]
  ///     preferred_gpu = 0
  int preferred_gpu_ = 0;

 protected:
  /// Assign values from config file to variables
  void AssignFromConfig(const std::shared_ptr<cpptoml::table>&);

 private:
  friend class Simulation;
  static std::unordered_map<ModuleParamUid, std::unique_ptr<ModuleParam>>
      registered_modules_;
  std::unordered_map<ModuleParamUid, ModuleParam*> modules_;
  BDM_CLASS_DEF_NV(Param, 1);
};

}  // namespace bdm

#endif  // CORE_PARAM_PARAM_H_
