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
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/param/param_group.h"
#include "core/util/root.h"
#include "core/util/type.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

class Simulation;

struct Param {
  static void RegisterParamGroup(ParamGroup* param);

  Param();

  ~Param();

  void Restore(Param&& other);

  /// Returns a Json representation of this parameter and all
  /// ParamGroupeter.
  /// The groups_ data member has been flattened to simplify
  /// JSON merge patches (https://tools.ietf.org/html/rfc7386).
  std::string ToJsonString() const;

  /// Applies a JSON merge patch (https://tools.ietf.org/html/rfc7386)
  /// to this parameter and ParamGroupeter.
  /// The groups_ data member must be flattened. See output of
  /// `ToJsonString()`.
  void MergeJsonPatch(const std::string& patch);

  template <typename TParamGroup>
  const TParamGroup* Get() const {
    assert(groups_.find(TParamGroup::kUid) != groups_.end() &&
           "Couldn't find the requested group parameter.");
    return bdm_static_cast<const TParamGroup*>(groups_.at(TParamGroup::kUid));
  }

  template <typename TParamGroup>
  TParamGroup* Get() {
    assert(groups_.find(TParamGroup::kUid) != groups_.end() &&
           "Couldn't find the requested group parameter.");
    return bdm_static_cast<TParamGroup*>(groups_.at(TParamGroup::kUid));
  }

  // simulation values ---------------------------------------------------------
  /// Set random number seed.\n
  /// The pseudo random number generator (prng) of each thread will be
  /// initialized as follows:
  /// `prng[tid].SetSeed(random_seed * (tid + 1));`\n
  /// Default value: `4357`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     random_seed = 4357
  uint64_t random_seed = 4357;

  /// Variable which specifies method using for solving differential equation
  /// {"Euler", "RK4"}.
  enum NumericalODESolver { kEuler = 1, kRK4 = 2 };
  NumericalODESolver numerical_ode_solver = NumericalODESolver::kEuler;

  /// Ouput Directory name used to store visualization and other files.\n
  /// Path is relative to working directory.\n
  /// Default value: `"output"`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     output_dir = "output"
  std::string output_dir = "output";

  /// If set to true, BioDynaMo will automatically delete all contents
  /// inside `Param::output_dir` at the beginning of the simulation.
  /// Use with caution, especially in combination with `Param::output_dir`
  bool remove_output_dir_contents = false;

  /// Backup file name for full simulation backups\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no backups will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_file = <path>/<filename>.root
  /// Command line argument: `-b, --backup`
  std::string backup_file = "";

  /// File name to restore simulation from\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no restore will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     restore_file = <path>/<filename>.root
  /// Command line argument: `-r, --restore`
  std::string restore_file = "";

  /// Specifies the interval (in seconds) in which backups will be performed.\n
  /// Default Value: `1800` (every half an hour)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_interval = 1800  # backup every half an hour
  uint32_t backup_interval = 1800;

  /// Time between two simulation steps, in hours.
  /// Default value: `0.01`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     time_step = 0.0125
  double simulation_time_step = 0.01;

  /// Maximum jump that a point mass can do in one time step. Useful to
  /// stabilize the simulation\n
  /// Default value: `3.0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_displacement = 3.0
  double simulation_max_displacement = 3.0;

  /// Calculate mechanical interactions between agents.\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     run_mechanical_interactions = true
  bool run_mechanical_interactions = true;

  /// Enforce an artificial cubic bounds around the simulation space.
  /// Agents cannot move outside this cube. Dimensions of this cube
  /// are determined by parameter `lbound` and `rbound`.\n
  /// Default value: `false` (simulation space is "infinite")\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     bound_space = false
  bool bound_space = false;

  /// Minimum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space`).\n
  /// Default value: `0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     min_bound = 0
  double min_bound = 0;

  /// Maximum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space`).\n
  /// Default value: `100`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_bound = 100
  double max_bound = 100;

  /// Allow substances to leak out of the simulation space. In this way
  /// the substance concentration will not be blocked by an artificial border\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     leaking_edges = true
  bool leaking_edges = true;

  /// A string for determining diffusion type within the simulation space.
  /// current inputs include "Euler" and Runga Kutta ("RK")
  /// Default value: `"Euler"`\n
  /// TOML config file:
  ///
  ///        [simulation]
  ///        diffusion_type = <diffusion method>
  ///

  std::string diffusion_type = "Euler";

  /// Calculate the diffusion gradient for each substance.\n
  /// TOML config file:
  /// Default value: `true`\n
  ///
  ///     [simulation]
  ///     calculate_gradients = true
  bool calculate_gradients = true;

  /// List of thread-safety mechanisms \n
  /// `kNone`: \n
  /// `kUserSpecified`: The user has to define all agent that must
  /// not be processed in parallel. \see `Agent::CriticalRegion`.\n
  /// `kAutomatic`: The simulation automatically locks all agents
  /// of the microenvironment.
  enum ThreadSafetyMechanism { kNone = 0, kUserSpecified, kAutomatic };

  /// Select the thread-safety mechanism.\n
  /// Possible values are: none, user-specified, automatic.\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     thread_safety_mechanism = "none"
  ThreadSafetyMechanism thread_safety_mechanism =
      ThreadSafetyMechanism::kUserSpecified;

  // visualization values ------------------------------------------------------

  /// Name of the visualization engine to use for visualizaing BioDynaMo
  /// simulations\n
  /// Default value: `"paraview"`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     adaptor = <name_of_adaptor>
  std::string visualization_engine = "paraview";

  /// Use ParaView Catalyst for insitu visualization.\n
  /// Insitu visualization supports live visualization
  /// and rendering without writing files to the harddisk.\n
  ///
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     insitu = false
  bool insitu_visualization = false;

  /// Write data to file for post-simulation visualization
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export = false
  bool export_visualization = false;

  /// Use ROOT for enable visualization.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     root = false
  bool root_visualization = false;

  /// Enable insitu visualization with a custom python pipeline
  /// Default value:
  /// `"<path-to-bdm>/include/core/visualization/paraview/default_insitu_pipeline.py"`\n
  /// TOML config file:
  ///     [visualization]
  ///     pv_insitu_pipeline = ""
  std::string pv_insitu_pipeline =
      Concat(std::getenv("BDMSYS"),
             "/include/core/visualization/paraview/default_insitu_pipeline.py");

  /// Arguments that will be passed to the python ParaView insitu pipeline
  /// specified in `Param::pv_insitu_pipeline`.\n
  /// The arguments will be passed to the ExtendDefaultPipeline function
  /// `def ExtendDefaultPipeline(renderview, coprocessor, datadescription,
  /// script_args):`
  /// as fourth argument.\n
  /// Default value: ""\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     pv_insitu_pipelinearguments = ""
  std::string pv_insitu_pipelinearguments = "";

  /// If `export_visualization` is set to true, this parameter specifies
  /// how often it should be exported. 1 = every timestep, 10: every 10
  /// time steps.\n
  /// Default value: `1`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     interval = 1
  uint32_t visualization_interval = 1;

  /// If `export_visualization` is set to true, this parameter specifies
  /// if the ParaView pvsm file will be generated!\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export_generate_pvsm = true
  bool visualization_export_generate_pvsm = true;

  /// Specifies which agents should be visualized. \n
  /// Every agent defines the minimum set of data members which
  /// are required to visualize it. (e.g. Cell: `position_` and `diameter_`).\n
  /// With this parameter it is also possible to extend the number of data
  /// members that are sent to the visualization engine.
  /// Default value: empty (no agent will be visualized)\n
  /// NB: This data member is not backed up, due to a ROOT error.
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on insitu or export
  ///     export = true
  ///
  ///       [[visualize_agent]]
  ///       name = "Cell"
  ///       # the following entry is optional
  ///       additional_data_members = [ "density_" ]
  ///
  ///       # The former block can be repeated for further agents
  ///       [[visualize_agent]]
  ///       name = "Neurite"
  std::map<std::string, std::set<std::string>>
      visualize_agents;  ///<  JSON_object

  struct VisualizeDiffusion {
    std::string name;
    bool concentration = true;
    bool gradient = false;
  };

  /// Specifies for which substances extracellular diffusion should be
  /// visualized.\n
  /// Default value: empty (no diffusion will be visualized)\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on insitu or export
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
  std::vector<VisualizeDiffusion> visualize_diffusion;

  /// Specifies if the ParView files that are generated in export mode
  /// should be compressed.\n
  /// Default value: true\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export = true
  ///     compress_pv_files = true
  ///
  bool visualization_compress_pv_files = true;

  // performance values --------------------------------------------------------

  /// Batch size used by the `Scheduler` to iterate over agents\n
  /// Default value: `1000`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     scheduling_batch_size = 1000
  uint64_t scheduling_batch_size = 1000;

  /// Calculation of the displacement (mechanical interaction) is an
  /// expensive operation. If agents do not move or grow,
  /// displacement calculation is ommited if detect_static_agents is turned
  /// on. However, the detection mechanism introduces an overhead. For dynamic
  /// simulations where agents move and grow, the overhead outweighs the
  /// benefits.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     detect_static_agents = false
  bool detect_static_agents = false;

  /// Neighbors of an agent can be cached so to avoid consecutive
  /// searches. This of course only makes sense if there is more than one
  /// `ForEachNeighbor*` operation.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     cache_neighbors = false
  bool cache_neighbors = false;

  /// If the utilization in the AgentUidMap inside ResourceManager falls below
  /// this watermark, defragmentation will be turned on.\n
  /// Default value: `0.5`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     agent_uid_defragmentation_low_watermark = 0.5
  double agent_uid_defragmentation_low_watermark = 0.5;

  /// If the utilization in the AgentUidMap inside ResourceManager rises above
  /// this watermark, defragmentation will be turned off.\n
  /// Default value: `0.9`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     agent_uid_defragmentation_high_watermark = 0.9
  double agent_uid_defragmentation_high_watermark = 0.9;

  /// Use the BioDynaMo memory manager.
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     use_bdm_mem_mgr = true
  bool use_bdm_mem_mgr = true;

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
  uint64_t mem_mgr_aligned_pages_shift = 8;

  /// The BioDynaMo memory manager allocates memory in increasing sizes using
  /// a geometric series. This parameter specifies the growth rate.
  /// Default value: `2.0`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_growth_rate = 1.1
  double mem_mgr_growth_rate = 1.1;

  /// The BioDynaMo memory manager can migrate memory between thread pools
  /// to avoid memory leaks.\n
  /// This parameter specifies the maximum memory size in bytes before
  /// migration happens.\n
  /// This value must be bigger than `PAGE_SIZE * 2 ^ mem_mgr_growth_rate`\n
  /// Default value: `10485760` (10 MB)\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_max_mem_per_thread = 10485760
  uint64_t mem_mgr_max_mem_per_thread = 1024 * 1024 * 10;

  /// This parameter is used inside `ResourceManager::LoadBalance`.
  /// If it is set to true, the function will reuse existing memory to rebalance
  /// agents to NUMA nodes. (A small amount of additional memory
  /// is still required.)\n
  /// If this parameter is set to false, the balancing function will first
  /// create new objects and delete the old ones in a second step. In the worst
  /// case this will double the required memory for agents for.
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     minimize_memory_while_rebalancing = true
  bool minimize_memory_while_rebalancing = true;

  /// MappedDataArrayMode options:
  ///   `kZeroCopy`: access agent data directly only if it is
  ///                requested. \n
  ///   `kCache`:    Like `kZeroCopy` but stores the results in contigous
  ///                array, to speed up access if it is used again.\n
  ///   `kCopy`:     Copy all data elements to a contigous array at
  ///                initialization time. Serves requests from the cache.
  enum MappedDataArrayMode { kZeroCopy = 0, kCopy, kCache };

  /// This parameter sets the operation mode in `bdm::MappedDataArray`.\n
  /// Allowed values are defined in `MappedDataArrayMode`\n
  /// Possible values: zero-copy, cache, copy\n
  /// Default value: `zero-copy`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mapped_data_array_mode = "zero-copy"
  Param::MappedDataArrayMode mapped_data_array_mode =
      MappedDataArrayMode::kZeroCopy;

  // development values --------------------------------------------------------
  /// Statistics of profiling data; keeps track of the execution time of each
  /// operation at every timestep.\n
  /// If set to true it prints simulation data at the end of the simulation
  /// to std::cout and a file.\n
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     statistics = false
  bool statistics = false;

  /// Output debugging info related to running on NUMA architecture.\n
  /// \see `ThreadInfo`, `ResourceManager::DebugNuma`
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     debug_numa = false
  bool debug_numa = false;

  /// Display the current simulation step in the terminal output
  /// Default value: `true`\n
  /// TOML config file:
  ///     [development]
  ///     show_simulation_step = true
  bool show_simulation_step = true;

  /// Sets the frequency at which the current simulation step is displayed.
  /// Display every `simulation_step_freq` steps.
  /// Default value: `10`\n
  /// TOML config file:
  ///     [development]
  ///     simulation_step_freq = false
  uint32_t simulation_step_freq = 10;

  // ---------------------------------------------------------------------------
  // experimental group

  /// Run the simulation partially on the GPU for improved performance.
  /// Possible values: "cpu", "cuda", "opencl"
  /// Default value: `"cpu"`\n
  /// TOML config file:
  ///     [experimental]
  ///     compute_target = false
  std::string compute_target = "cpu";

  /// Compile OpenCL kernels with debugging symbols, for debugging on CPU
  /// targets with GNU gdb.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [experimental]
  ///     opencl_debug = false
  bool opencl_debug = false;

  /// Set the index of the preferred GPU you wish to use.
  /// Default value: `0`\n
  /// TOML config file:
  ///     [experimental]
  ///     preferred_gpu = 0
  int preferred_gpu = 0;

  /// Assign values from config file to variables
  void AssignFromConfig(const std::shared_ptr<cpptoml::table>&);

 private:
  static std::unordered_map<ParamGroupUid, std::unique_ptr<ParamGroup>>
      registered_groups_;
  std::unordered_map<ParamGroupUid, ParamGroup*> groups_;
  BDM_CLASS_DEF_NV(Param, 1);
};

}  // namespace bdm

#endif  // CORE_PARAM_PARAM_H_
