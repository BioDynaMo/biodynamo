// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include <TBufferJSON.h>
#include <TSystem.h>
#include <json.hpp>

#include <utility>
#include <vector>

#include "core/multi_simulation/optimization_param.h"
#include "core/param/param.h"
#include "core/util/cpptoml.h"
#include "core/util/log.h"

using nlohmann::json;

namespace bdm {

const bdm::ParamGroupUid bdm::OptimizationParam::kUid =
    bdm::ParamGroupUidGenerator::Get()->NewUid();

std::unordered_map<ParamGroupUid, std::unique_ptr<ParamGroup>>
    Param::registered_groups_;

// -----------------------------------------------------------------------------
void Param::RegisterParamGroup(ParamGroup* param) {
  registered_groups_[param->GetUid()] = std::unique_ptr<ParamGroup>(param);
}

// -----------------------------------------------------------------------------
Param::Param() {
  RegisterParamGroup(new OptimizationParam());
  for (auto& el : registered_groups_) {
    groups_[el.first] = el.second->NewCopy();
  }
}

// -----------------------------------------------------------------------------
Param::~Param() {
  for (auto& el : groups_) {
    delete el.second;
  }
}

// -----------------------------------------------------------------------------
void Param::Restore(Param&& other) {
  for (auto& el : groups_) {
    delete el.second;
  }
  *this = other;
  other.groups_.clear();
}

Param::Param(const Param& other) {
  *this = other;
  for (auto el : other.groups_) {
    this->groups_[el.first] = el.second->NewCopy();
  }
}

// -----------------------------------------------------------------------------
json FlattenGroups(const json& j_document) {
  json j_copy = j_document;
  j_copy.erase("groups_");

  json j_new;
  j_new["bdm::Param"] = j_copy;

  // iterator over all group parameters
  auto j_groups = j_document["groups_"];
  for (json::iterator it = j_groups.begin(); it != j_groups.end(); ++it) {
    j_new[(*it)["second"]["_typename"].get<std::string>()] = (*it)["second"];
  }
  return j_new;
}

// -----------------------------------------------------------------------------
json UnflattenGroups(const json& j_flattened, const json& j_original) {
  json j_return = j_flattened["bdm::Param"];
  j_return["groups_"] = {};
  auto& j_groups = j_return["groups_"];

  auto j_original_groups = j_original["groups_"];
  for (json::iterator it = j_original_groups.begin();
       it != j_original_groups.end(); ++it) {
    json j_param_group;
    j_param_group["$pair"] = (*it)["$pair"];
    j_param_group["first"] = (*it)["first"];
    j_param_group["second"] =
        j_flattened[(*it)["second"]["_typename"].get<std::string>()];
    j_groups.push_back(j_param_group);
  }
  return j_return;
}

// -----------------------------------------------------------------------------
std::string Param::ToJsonString() const {
  // If you segfault here, try running the unit tests to find the root cause
  std::string current_json_str(
      TBufferJSON::ToJSON(this, TBufferJSON::kMapAsObject).Data());
  // Flatten groups_ to simplify json patches in rfc7386 format.
  try {
    json j_document = json::parse(current_json_str);
    auto j_flattened = FlattenGroups(j_document);
    return j_flattened.dump(4);
  } catch (std::exception& e) {
    Log::Fatal("Param::ToJsonString",
               Concat("Couldn't parse `Param` parameters.\n", e.what(), "\n",
                      current_json_str));
    return std::string();
  }
}

// -----------------------------------------------------------------------------
void Param::MergeJsonPatch(const std::string& patch) {
  // If you segfault here, try running the unit tests to find the root cause
  std::string json_str(
      TBufferJSON::ToJSON(this, TBufferJSON::kMapAsObject).Data());
  json j_param = json::parse(json_str);
  auto j_flattened = FlattenGroups(j_param);

  auto j_patch = json::parse(patch);
  try {
    j_flattened.merge_patch(j_patch);
  } catch (std::exception& e) {
    Log::Fatal("Param::MergeJsonPatch",
               Concat("Couldn't merge the given json parameters.\n", e.what(),
                      "\n", j_patch));
  }

  auto j_unflattened = UnflattenGroups(j_flattened, j_param);
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
      param->thread_safety_mechanism = Param::ThreadSafetyMechanism::kNone;
    } else if (str_value == "user-specified") {
      param->thread_safety_mechanism =
          Param::ThreadSafetyMechanism::kUserSpecified;
    } else if (str_value == "automatic") {
      param->thread_safety_mechanism = Param::ThreadSafetyMechanism::kAutomatic;
    }
  }
}

// -----------------------------------------------------------------------------
void AssignMappedDataArrayMode(const std::shared_ptr<cpptoml::table>& config,
                               Param* param) {
  const std::string config_key = "performance.mapped_data_array_mode";
  if (config->contains_qualified(config_key)) {
    auto value = config->get_qualified_as<std::string>(config_key);
    if (!value) {
      return;
    }
    auto str_value = *value;
    if (str_value == "zero-copy") {
      param->mapped_data_array_mode = Param::MappedDataArrayMode::kZeroCopy;
    } else if (str_value == "cache") {
      param->mapped_data_array_mode = Param::MappedDataArrayMode::kCache;
    } else if (str_value == "copy") {
      param->mapped_data_array_mode = Param::MappedDataArrayMode::kCopy;
    } else {
      Log::Fatal(
          "Param",
          Concat(
              "Parameter mapped_data_array_mode was set to an invalid value (",
              str_value, ")."));
    }
  }
}

// -----------------------------------------------------------------------------
void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  // group parameters
  for (auto& el : groups_) {
    el.second->AssignFromConfig(config);
  }

  // simulation group
  BDM_ASSIGN_CONFIG_VALUE(random_seed, "simulation.random_seed");
  BDM_ASSIGN_CONFIG_VALUE(output_dir, "simulation.output_dir");
  BDM_ASSIGN_CONFIG_VALUE(backup_file, "simulation.backup_file");
  BDM_ASSIGN_CONFIG_VALUE(restore_file, "simulation.restore_file");
  BDM_ASSIGN_CONFIG_VALUE(backup_interval, "simulation.backup_interval");
  BDM_ASSIGN_CONFIG_VALUE(simulation_time_step, "simulation.time_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_max_displacement,
                          "simulation.max_displacement");
  BDM_ASSIGN_CONFIG_VALUE(bound_space, "simulation.bound_space");
  BDM_ASSIGN_CONFIG_VALUE(min_bound, "simulation.min_bound");
  BDM_ASSIGN_CONFIG_VALUE(max_bound, "simulation.max_bound");
  BDM_ASSIGN_CONFIG_VALUE(diffusion_boundary_condition,
                          "simulation.diffusion_boundary_condition");
  BDM_ASSIGN_CONFIG_VALUE(diffusion_method, "simulation.diffusion_method");
  BDM_ASSIGN_CONFIG_VALUE(calculate_gradients,
                          "simulation.calculate_gradients");
  AssignThreadSafetyMechanism(config, this);

  // visualization group
  BDM_ASSIGN_CONFIG_VALUE(visualization_engine, "visualization.adaptor");
  BDM_ASSIGN_CONFIG_VALUE(insitu_visualization, "visualization.insitu");
  BDM_ASSIGN_CONFIG_VALUE(pv_insitu_pipeline,
                          "visualization.pv_insitu_pipeline");
  BDM_ASSIGN_CONFIG_VALUE(pv_insitu_pipelinearguments,
                          "visualization.pv_insitu_pipelinearguments");
  BDM_ASSIGN_CONFIG_VALUE(root_visualization, "visualization.root");
  BDM_ASSIGN_CONFIG_VALUE(export_visualization, "visualization.export");
  BDM_ASSIGN_CONFIG_VALUE(visualization_interval, "visualization.interval");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_generate_pvsm,
                          "visualization.export_generate_pvsm");
  BDM_ASSIGN_CONFIG_VALUE(visualization_compress_pv_files,
                          "visualization.compress_pv_files");

  //   visualize_agents
  auto visualize_agentstarr = config->get_table_array("visualize_agent");
  if (visualize_agentstarr) {
    for (const auto& table : *visualize_agentstarr) {
      // We do a 'redundant' check here, because `get_as` on Mac OS does not
      // catch the exception when the "name" is not defined in the bdm.toml
      // Same goes for all the other redundant checks
      if (table->contains("name")) {
        auto name = table->get_as<std::string>("name");
        if (!name) {
          Log::Warning("AssignFromConfig",
                       "Missing name for attribute visualize_agent");
          continue;
        }

        if (table->contains("additional_data_members")) {
          auto dm_option =
              table->get_array_of<std::string>("additional_data_members");

          std::set<std::string> data_members;
          for (const auto& val : *dm_option) {
            data_members.insert(val);
          }
          visualize_agents[*name] = data_members;
        } else {
          std::set<std::string> data_members;
          visualize_agents[*name] = data_members;
        }
      }
    }
  }

  //   visualize_diffusion
  auto visualize_diffusiontarr = config->get_table_array("visualize_diffusion");
  if (visualize_diffusiontarr) {
    for (const auto& table : *visualize_diffusiontarr) {
      if (table->contains("name")) {
        auto name = table->get_as<std::string>("name");
        if (!name) {
          Log::Warning("AssignFromConfig",
                       "Missing name for attribute visualize_diffusion");
          continue;
        }

        VisualizeDiffusion vd;
        vd.name = *name;

        if (table->contains("concentration")) {
          auto concentration = table->get_as<bool>("concentration");
          if (concentration) {
            vd.concentration = *concentration;
          }
        }
        if (table->contains("gradient")) {
          auto gradient = table->get_as<bool>("gradient");
          if (gradient) {
            vd.gradient = *gradient;
          }
        }

        visualize_diffusion.push_back(vd);
      }
    }
  }

  // unschedule_default_operations
  if (config->get_table("simulation")) {
    auto disabled_ops =
        config->get_table("simulation")
            ->get_array_of<std::string>("unschedule_default_operations");
    for (const auto& op : *disabled_ops) {
      unschedule_default_operations.push_back(op);
    }
  }

  // performance group
  BDM_ASSIGN_CONFIG_VALUE(scheduling_batch_size,
                          "performance.scheduling_batch_size");
  BDM_ASSIGN_CONFIG_VALUE(detect_static_agents,
                          "performance.detect_static_agents");
  BDM_ASSIGN_CONFIG_VALUE(cache_neighbors, "performance.cache_neighbors");
  BDM_ASSIGN_CONFIG_VALUE(
      agent_uid_defragmentation_low_watermark,
      "performance.agent_uid_defragmentation_low_watermark");
  BDM_ASSIGN_CONFIG_VALUE(
      agent_uid_defragmentation_high_watermark,
      "performance.agent_uid_defragmentation_high_watermark");
  BDM_ASSIGN_CONFIG_VALUE(use_bdm_mem_mgr, "performance.use_bdm_mem_mgr");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_aligned_pages_shift,
                          "performance.mem_mgr_aligned_pages_shift");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_growth_rate,
                          "performance.mem_mgr_growth_rate");
  BDM_ASSIGN_CONFIG_VALUE(mem_mgr_max_mem_per_thread,
                          "performance.mem_mgr_max_mem_per_thread");
  BDM_ASSIGN_CONFIG_VALUE(minimize_memory_while_rebalancing,
                          "performance.minimize_memory_while_rebalancing");
  AssignMappedDataArrayMode(config, this);

  // development group
  BDM_ASSIGN_CONFIG_VALUE(statistics, "development.statistics");
  BDM_ASSIGN_CONFIG_VALUE(debug_numa, "development.debug_numa");
  BDM_ASSIGN_CONFIG_VALUE(show_simulation_step,
                          "development.show_simulation_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_step_freq,
                          "development.simulation_step_freq");

  // experimental group
  BDM_ASSIGN_CONFIG_VALUE(compute_target, "experimental.compute_target");
  BDM_ASSIGN_CONFIG_VALUE(opencl_debug, "experimental.opencl_debug");
  BDM_ASSIGN_CONFIG_VALUE(preferred_gpu, "experimental.preferred_gpu");
}

}  // namespace bdm
