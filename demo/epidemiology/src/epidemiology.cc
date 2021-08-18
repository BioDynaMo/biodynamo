// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "epidemiology.h"
#include "analytical-solution.h"
#include "core/multi_simulation/database.h"
#include "core/multi_simulation/multi_simulation.h"
#include "sim-param.h"

using namespace bdm;

const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

TimeSeries GetAnalyticalResults(CommandLineOptions* clo) {
  // Create simulation object just to obtain parameter values
  Simulation sim(clo, [](Param* param) { param->statistics = false; });
  auto* sparam = sim.GetParam()->Get<SimParam>();

  // analytical solution
  double beta = clo->Get<double>("beta");
  double gamma = clo->Get<double>("gamma");
  TimeSeries analytical;
  CalculateAnalyticalSolution(
      &analytical, beta, gamma, sparam->initial_population_susceptible,
      sparam->initial_population_infected, 0, sparam->number_of_iterations, 1);
  return analytical;
}

int main(int argc, const char** argv) {
  // register parameters that are specific for this simulation
  Param::RegisterParamGroup(new SimParam());

  // define additional command line options
  CommandLineOptions clo(argc, argv);
  clo.AddOption<std::string>("mode", "sim-and-analytical");
  clo.AddOption<double>("beta", "0.06719");
  clo.AddOption<double>("gamma", "0.00521");
  clo.AddOption<uint64_t>("repeat", "10");
  clo.AddOption<bool>("no-legend", "false");

  // Generate the analytical data
  auto analytical = GetAnalyticalResults(&clo);

  experimental::MultiSimulation pe(argc, argv, analytical);
  return pe.Execute(Simulate);
}
