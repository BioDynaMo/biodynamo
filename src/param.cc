#include "param.h"
#include <TError.h>

namespace bdm {

// simulation group
std::string Param::backup_file_ = "";
std::string Param::restore_file_ = "";
uint32_t Param::backup_interval_ = 1800;
double Param::simulation_time_step_ = 0.01;
double Param::simulation_max_displacement_ = 3.0;
bool Param::run_mechanical_interactions_ = true;
bool Param::bound_space_ = false;
double Param::min_bound_ = 0;
double Param::max_bound_ = 100;

// visualization group
bool Param::live_visualization_ = false;
bool Param::export_visualization_ = false;
uint32_t Param::visualization_export_interval_ = 1;
std::unordered_map<std::string, std::set<std::string>> Param::visualize_;

// development group
bool Param::output_op_runtime_ = false;

#define BDM_ASSIGN_CONFIG_VALUE(variable, config_key)                      \
  {                                                                        \
    auto value = config->get_qualified_as<decltype(variable)>(config_key); \
    if (value) {                                                           \
      variable = *value;                                                   \
    }                                                                      \
  }

void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  // simulation group
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
  // visualization group
  BDM_ASSIGN_CONFIG_VALUE(live_visualization_, "visualization.live");
  BDM_ASSIGN_CONFIG_VALUE(export_visualization_, "visualization.export");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_interval_,
                          "visualization.export_interval");

  //   visualize
  auto visualize_tarr = config->get_table_array("visualize");
  for (const auto& table : *visualize_tarr) {
    auto name = table->get_as<std::string>("name");
    if (!name) {
      Warning("AssignFromConfig", "Missing name for attribute visualize");
      continue;
    }
    auto dm_option =
        table->get_array_of<std::string>("additional_data_members");

    std::set<std::string> data_members;
    for (const auto& val : *dm_option) {
      data_members.insert(val);
    }
    visualize_[*name] = data_members;
  }

  // development group
  BDM_ASSIGN_CONFIG_VALUE(output_op_runtime_, "development.output_op_runtime");
}

void Param::Reset() {
  // simulation group
  backup_file_ = "";
  restore_file_ = "";
  backup_interval_ = 1800;
  simulation_time_step_ = 0.01;
  simulation_max_displacement_ = 3.0;
  run_mechanical_interactions_ = true;
  bound_space_ = false;
  min_bound_ = 0;
  max_bound_ = 100;

  // visualization group
  live_visualization_ = false;
  export_visualization_ = false;
  visualization_export_interval_ = 1;
  visualize_.clear();

  // development group
  output_op_runtime_ = false;
}

}  // namespace bdm
