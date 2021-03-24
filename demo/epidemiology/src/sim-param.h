// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include "core/param/param_group.h"

namespace bdm {

/// This class defines parameters that are specific to this simulation.
/// The default values are set to simulate a measles outbreak.
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  std::string result_plot = "result";
  uint64_t number_of_iterations = 1000;
  uint64_t initial_population_susceptible = 2000;
  uint64_t initial_population_infected = 10;
  double infection_radius = 10.5092197414493;
  double moving_agents_ratio = 1.0;
  double recovery_probability = 0.00521;
  double agent_diameter = 2.0;
  double agent_speed = 5.78594372145249;
  double infection_probablity = 0.285097276954021;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
