// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include <TStyle.h>
#include "core/param/param_group.h"

namespace bdm {

/// This class defines parameters that are specific to this simulation.
/// The default values are set to simulate a measles outbreak.
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  std::string mode = "sim-and-analytical";
  real_t beta = 0.06719;
  real_t gamma = 0.00521;
  uint64_t repeat = 10;
  bool no_legend = false;

  std::string result_plot = "result";
  uint64_t number_of_iterations = 1000;
  uint64_t initial_population_susceptible = 2000;
  uint64_t initial_population_infected = 10;
  real_t infection_radius = 10.5092197414493;
  real_t moving_agents_ratio = 1.0;
  real_t recovery_probability = 0.00521;
  real_t agent_diameter = 2.0;
  real_t agent_speed = 5.78594372145249;
  real_t infection_probablity = 0.285097276954021;
  experimental::Style root_style;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
