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

#include "core/param/param.h"
#include <utility>
#include <vector>
#include <TBufferJSON.h>
#include <json.hpp>
#include "core/util/cpptoml.h"
#include "core/util/log.h"

using nlohmann::json;

namespace bdm {

std::unordered_map<ModuleParamUid, std::unique_ptr<ModuleParam>>
    Param::registered_modules_;

// -----------------------------------------------------------------------------
void Param::RegisterModuleParam(ModuleParam* param) {
  registered_modules_[param->GetUid()] = std::unique_ptr<ModuleParam>(param);
}

// -----------------------------------------------------------------------------
Param::Param() {
  for (auto& el : registered_modules_) {
    modules_[el.first] = el.second->GetCopy();
  }
}

// -----------------------------------------------------------------------------
Param::~Param() {
  for (auto& el : modules_) {
    delete el.second;
  }
}

// -----------------------------------------------------------------------------
void Param::Restore(Param&& other) {
  for (auto& el : modules_) {
    delete el.second;
  }
  *this = other;
  other.modules_.clear();
}

// -----------------------------------------------------------------------------
json FlattenModules(const json& j_document) {
  json j_copy = j_document;
  j_copy.erase("modules_");

  json j_new;
  j_new["bdm::Param"] = j_copy;

  // iterator over all module parameters
  auto j_modules = j_document["modules_"];
  for (json::iterator it = j_modules.begin(); it != j_modules.end(); ++it) {
    j_new[(*it)["second"]["_typename"].get<std::string>()] = (*it)["second"];
  }
  return j_new;
}

// -----------------------------------------------------------------------------
json UnflattenModules(const json& j_flattened, const json& j_original) {
  json j_return = j_flattened["bdm::Param"];
  j_return["modules_"] = {};
  auto& j_modules = j_return["modules_"];

  auto j_original_modules = j_original["modules_"];
  for (json::iterator it = j_original_modules.begin();
       it != j_original_modules.end(); ++it) {
    json j_module_param;
    j_module_param["$pair"] = (*it)["$pair"];
    j_module_param["first"] = (*it)["first"];
    j_module_param["second"] =
        j_flattened[(*it)["second"]["_typename"].get<std::string>()];
    j_modules.push_back(j_module_param);
  }
  return j_return;
}

// -----------------------------------------------------------------------------
std::string Param::ToJsonString() const {
  std::string current_json_str(
      TBufferJSON::ToJSON(this, TBufferJSON::kMapAsObject).Data());
  // Flatten modules_ to simplify json patches in rfc7386 format.
  json j_document = json::parse(current_json_str);
  auto j_flattened = FlattenModules(j_document);
  return j_flattened.dump(4);
}

// -----------------------------------------------------------------------------
void Param::MergeJsonPatch(const std::string& patch) {
  std::string json_str(
      TBufferJSON::ToJSON(this, TBufferJSON::kMapAsObject).Data());
  json j_param = json::parse(json_str);
  auto j_flattened = FlattenModules(j_param);

  try {
    auto j_patch = json::parse(patch);
    j_flattened.merge_patch(j_patch);
  } catch (std::exception& e) {
    Log::Fatal("Param::RestoreFromJson",
               Concat("Couldn't parse or merge the given json parameters.\n",
                      e.what(), "\n", patch));
  }

  auto j_unflattened = UnflattenModules(j_flattened, j_param);
  Param* restored = nullptr;
  TBufferJSON::FromJSON(restored, j_unflattened.dump().c_str());
  Restore(std::move(*restored));
}

// -----------------------------------------------------------------------------
void AssignThreadSafetyMechanism(const std::shared_ptr<cpptoml::table>& config,
                                 Param* param) {
  const std::string config_key = "simulation.thread_safety_mechanism";
  if (config->contains_qualified(config_key)) {
    auto value = config->get_qualified_as<std::string>(config_key);
    if (!value) {
      return;
    }
    auto str_value = *value;
    if (str_value == "none") {
      param->thread_safety_mechanism_ = Param::ThreadSafetyMechanism::kNone;
    } else if (str_value == "user-specified") {
      param->thread_safety_mechanism_ =
          Param::ThreadSafetyMechanism::kUserSpecified;
    } else if (str_value == "automatic") {
      param->thread_safety_mechanism_ =
          Param::ThreadSafetyMechanism::kAutomatic;
    }
  }
}

// -----------------------------------------------------------------------------
void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  // module parameters
  for (auto& el : modules_) {
    el.second->AssignFromConfig(config);
  }

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
  BDM_ASSIGN_CONFIG_VALUE(diffusion_type_, "simulation.diffusion_type");
  BDM_ASSIGN_CONFIG_VALUE(calculate_gradients_,
                          "simulation.calculate_gradients");
  AssignThreadSafetyMechanism(config, this);

  // visualization group
  BDM_ASSIGN_CONFIG_VALUE(visualization_engine_, "visualization.adaptor");
  BDM_ASSIGN_CONFIG_VALUE(live_visualization_, "visualization.live");
  BDM_ASSIGN_CONFIG_VALUE(root_visualization_, "visualization.root");
  BDM_ASSIGN_CONFIG_VALUE(export_visualization_, "visualization.export");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_interval_,
                          "visualization.export_interval");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_generate_pvsm_,
                          "visualization.export_generate_pvsm");

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
  BDM_ASSIGN_CONFIG_VALUE(souid_defragmentation_low_watermark_,
                          "performance.souid_defragmentation_low_watermark");
  BDM_ASSIGN_CONFIG_VALUE(souid_defragmentation_high_watermark_,
                          "performance.souid_defragmentation_high_watermark");
  BDM_ASSIGN_CONFIG_VALUE(use_bdm_mem_mgr_, "performance.use_bdm_mem_mgr");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_aligned_pages_shift_,
                          "performance.mem_mgr_aligned_pages_shift");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_growth_rate_,
                          "performance.mem_mgr_growth_rate");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_max_mem_per_thread_,
                          "performance.mem_mgr_max_mem_per_thread");
  BDM_ASSIGN_CONFIG_VALUE(minimize_memory_while_rebalancing_,
                          "performance.minimize_memory_while_rebalancing");

  // development group
  BDM_ASSIGN_CONFIG_VALUE(statistics_, "development.statistics");
  BDM_ASSIGN_CONFIG_VALUE(debug_numa_, "development.debug_numa");
  BDM_ASSIGN_CONFIG_VALUE(python_paraview_pipeline_,
                          "development.python_paraview_pipeline");
  BDM_ASSIGN_CONFIG_VALUE(show_simulation_step_,
                          "development.show_simulation_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_step_freq_,
                          "development.simulation_step_freq");

  // experimental group
  BDM_ASSIGN_CONFIG_VALUE(compute_target_, "experimental.compute_target");
  BDM_ASSIGN_CONFIG_VALUE(opencl_debug_, "experimental.opencl_debug");
  BDM_ASSIGN_CONFIG_VALUE(preferred_gpu_, "experimental.preferred_gpu");
}

}  // namespace bdm
