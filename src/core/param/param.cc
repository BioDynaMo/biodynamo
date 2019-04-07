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

#include <vector>

#include "core/param/param.h"
#include "core/util/cpptoml.h"
#include "core/util/log.h"

namespace bdm {

void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  // simulation group
  BDM_ASSIGN_CONFIG_VALUE(output_dir_, "simulation.output_dir");
  BDM_ASSIGN_CONFIG_VALUE(backup_file_, "simulation.backup_file");
  BDM_ASSIGN_CONFIG_VALUE(restore_file_, "simulation.restore_file");
  BDM_ASSIGN_CONFIG_VALUE(backup_interval_, "simulation.backup_interval");
  BDM_ASSIGN_CONFIG_VALUE(simulation_time_step_, "simulation.time_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_max_displacement_,
                          "simulation.max_displacement");
  BDM_ASSIGN_CONFIG_VALUE(run_mechanical_interactions_,
                          "simulation.run_mechanical_interactions");
  BDM_ASSIGN_CONFIG_VALUE(bound_space_, "simulation.bound_space");
  BDM_ASSIGN_CONFIG_VALUE(min_bound_, "simulation.min_bound");
  BDM_ASSIGN_CONFIG_VALUE(max_bound_, "simulation.max_bound");
  BDM_ASSIGN_CONFIG_VALUE(leaking_edges_, "simulation.leaking_edges");
  BDM_ASSIGN_CONFIG_VALUE(calculate_gradients_,
                          "simulation.calculate_gradients");
  // visualization group
  BDM_ASSIGN_CONFIG_VALUE(live_visualization_, "visualization.live");
  BDM_ASSIGN_CONFIG_VALUE(export_visualization_, "visualization.export");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_interval_,
                          "visualization.export_interval");

  //   visualize_sim_objects_
  auto visualize_sim_objects_tarr =
      config->get_table_array("visualize_sim_object");
  if (visualize_sim_objects_tarr) {
    for (const auto& table : *visualize_sim_objects_tarr) {
      // We do a 'redundant' check here, because `get_as` on Mac OS does not
      // catch the exception when the "name" is not defined in the bdm.toml
      // Same goes for all the other redundant checks
      if (table->contains("name")) {
        auto name = table->get_as<std::string>("name");
        if (!name) {
          Log::Warning("AssignFromConfig",
                       "Missing name for attribute visualize_sim_object");
          continue;
        }

        if (table->contains("additional_data_members")) {
          auto dm_option =
              table->get_array_of<std::string>("additional_data_members");

          std::set<std::string> data_members;
          for (const auto& val : *dm_option) {
            data_members.insert(val);
          }
          visualize_sim_objects_[*name] = data_members;
        } else {
          std::set<std::string> data_members;
          visualize_sim_objects_[*name] = data_members;
        }
      }
    }
  }

  //   visualize_diffusion_
  auto visualize_diffusion_tarr =
      config->get_table_array("visualize_diffusion");
  if (visualize_diffusion_tarr) {
    for (const auto& table : *visualize_diffusion_tarr) {
      if (table->contains("name")) {
        auto name = table->get_as<std::string>("name");
        if (!name) {
          Log::Warning("AssignFromConfig",
                       "Missing name for attribute visualize_diffusion");
          continue;
        }

        VisualizeDiffusion vd;
        vd.name_ = *name;

        if (table->contains("concentration")) {
          auto concentration = table->get_as<bool>("concentration");
          if (concentration) {
            vd.concentration_ = *concentration;
          }
        }
        if (table->contains("gradient")) {
          auto gradient = table->get_as<bool>("gradient");
          if (gradient) {
            vd.gradient_ = *gradient;
          }
        }

        visualize_diffusion_.push_back(vd);
      }
    }
  }

  // performance group
  BDM_ASSIGN_CONFIG_VALUE(scheduling_batch_size_,
                          "performance.scheduling_batch_size");
  BDM_ASSIGN_CONFIG_VALUE(detect_static_sim_objects_,
                          "performance.detect_static_sim_objects");
  BDM_ASSIGN_CONFIG_VALUE(cache_neighbors_, "performance.cache_neighbors");

  // development group
  BDM_ASSIGN_CONFIG_VALUE(statistics_, "development.statistics");
  BDM_ASSIGN_CONFIG_VALUE(debug_numa_, "development.debug_numa");
  BDM_ASSIGN_CONFIG_VALUE(debug_exec_ctxt_caches_,
                          "development.debug_exec_ctxt_caches");
  BDM_ASSIGN_CONFIG_VALUE(python_catalyst_pipeline_,
                          "development.python_catalyst_pipeline");
  BDM_ASSIGN_CONFIG_VALUE(show_simulation_step_,
                          "development.show_simulation_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_step_freq_,
                          "development.simulation_step_freq");

  // experimental group
  BDM_ASSIGN_CONFIG_VALUE(use_gpu_, "experimental.use_gpu");
  BDM_ASSIGN_CONFIG_VALUE(use_opencl_, "experimental.use_opencl");
  BDM_ASSIGN_CONFIG_VALUE(opencl_debug_, "experimental.opencl_debug");
  BDM_ASSIGN_CONFIG_VALUE(preferred_gpu_, "experimental.preferred_gpu");
}

}  // namespace bdm
