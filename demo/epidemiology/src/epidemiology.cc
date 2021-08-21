// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "epidemiology.h"
#include "analytical-solution.h"
#include "sim-param.h"

#include "core/multi_simulation/database.h"
#include "core/multi_simulation/experiment.h"
#include "core/multi_simulation/multi_simulation.h"

using namespace bdm;
using bdm::experimental::MultiSimulation;

const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

TimeSeries GetAnalyticalResults(const Param* param) {
  // Create simulation object just to obtain parameter values
  auto* sparam = param->Get<SimParam>();

  // analytical solution
  double beta = sparam->beta;
  double gamma = sparam->gamma;
  TimeSeries analytical;
  CalculateAnalyticalSolution(
      &analytical, beta, gamma, sparam->initial_population_susceptible,
      sparam->initial_population_infected, 0, sparam->number_of_iterations, 1);
  return analytical;
}

void ExperimentSimAndAnalytical(int argc, const char** argv, const Param* param,
                                uint64_t repeat) {
  auto analytical = GetAnalyticalResults(param);

  auto sim_wrapper = L2F([&](Param* param, TimeSeries* result) {
    Simulate(argc, argv, result, param);
  });

  auto plot = L2F([&](const std::vector<TimeSeries>& results,
                      const TimeSeries& mean, const TimeSeries& analytical) {
    auto* sparam = param->Get<SimParam>();
    PlotResults(&analytical, &mean, results, sparam->root_style, "output",
                !sparam->no_legend, sparam->result_plot);
  });

  double mse = Experiment(sim_wrapper, repeat, param, &analytical, &plot);
  std::cout << " MSE " << mse << std::endl;
}

int main(int argc, const char** argv) {
  // register parameters that are specific for this simulation
  Param::RegisterParamGroup(new SimParam());
  Simulation simulation(argc, argv);
  auto* param = simulation.GetParam();
  auto* sparam = param->Get<SimParam>();

  auto repeat = sparam->repeat;

  // Run the simulation once and compute the error against the analytical
  // solution
  std::cout << sparam->mode << std::endl;
  if (sparam->mode == "sim-and-analytical") {
    ExperimentSimAndAnalytical(argc, argv, param, repeat);
    return 0;
  } else {  // Run the multi-simulation fitting routine
    // Generate the analytical data
    auto analytical = GetAnalyticalResults(param);

    MultiSimulation pe(argc, argv, analytical);
    return pe.Execute(Simulate);
  }
}
