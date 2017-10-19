#include "param.h"

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

// development group
bool Param::output_op_runtime_ = false;

#define BDM_ASSIGN_CONFIG_VALUE(variable, config_key)            \
  {                                                              \
    auto value = config->get_as<decltype(variable)>(config_key); \
    if (value) {                                                 \
      variable = *value;                                         \
    }                                                            \
  }

void Param::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {
  // simulation group
  BDM_ASSIGN_CONFIG_VALUE(backup_file_, "simulation.backup_file");
  BDM_ASSIGN_CONFIG_VALUE(restore_file_, "simulation.backup_file");
  BDM_ASSIGN_CONFIG_VALUE(backup_interval_, "simulation.backup_interval");
  BDM_ASSIGN_CONFIG_VALUE(simulation_time_step_, "simulation.time_step");
  BDM_ASSIGN_CONFIG_VALUE(simulation_max_displacement_,
                          "simulation.max_displacement");
  BDM_ASSIGN_CONFIG_VALUE(run_mechanical_interactions_,
                          "simulation.run_mechanical_interactions");
  BDM_ASSIGN_CONFIG_VALUE(bound_space_, "simulation.bound_space");
  BDM_ASSIGN_CONFIG_VALUE(min_bound_, "simulation.min_bound_");
  BDM_ASSIGN_CONFIG_VALUE(max_bound_, "simulation.max_bound_");
  // visualization group
  BDM_ASSIGN_CONFIG_VALUE(live_visualization_, "visualization.live");
  BDM_ASSIGN_CONFIG_VALUE(export_visualization_, "visualization.export");
  BDM_ASSIGN_CONFIG_VALUE(visualization_export_interval_,
                          "visualization.export_interval");
  // development group
  BDM_ASSIGN_CONFIG_VALUE(output_op_runtime_, "development.output_op_runtime");
}

}  // namespace bdm
