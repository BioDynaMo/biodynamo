// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "optim.hpp"

#include "epidemiology.h"
#include "sim-param.h"

namespace bdm {

const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

double Experiment(CommandLineOptions* clo, const std::vector<double>& seeds,
                  const TimeSeries& analytical, bool plot = false,
                  bool overwrite = false, double infection_probablity = 1,
                  double infection_radius = 1, double speed = 1) {
  std::vector<TimeSeries> results(seeds.size());
  for (uint64_t i = 0; i < seeds.size(); ++i) {
    Simulate(clo, seeds[i], &results[i], overwrite, infection_probablity,
             infection_radius, speed);
  }

  TimeSeries mean;
  TimeSeries::Merge(&mean, results,
                    [](const std::vector<double> all_y_values, double* y,
                       double* eh, double* el) {
                      *y =
                          TMath::Mean(all_y_values.begin(), all_y_values.end());
                    });
  double mse =
      Math::MSE(analytical.GetYValues("susceptible"),
                mean.GetYValues("susceptible")) +
      Math::MSE(analytical.GetYValues("infected"), mean.GetYValues("infected"));
  if (plot) {
    // Create simulation object just to obtain parameter values
    Simulation sim(clo, [](Param* param) { param->statistics = false; });
    auto* param = sim.GetParam();
    PlotResults(&analytical, &mean, results, param->root_style, "output",
                !clo->Get<bool>("no-legend"));
  }
  return mse;
}

TimeSeries GetAnalyticalResults(CommandLineOptions* clo) {
  // Create simulation object just to obtain parameter values
  Simulation sim(clo, [](Param* param) { param->statistics = false; });
  auto* param = sim.GetParam();
  auto* sparam = param->Get<SimParam>();

  // analytical solution
  double beta = clo->Get<double>("beta");
  double gamma = clo->Get<double>("gamma");
  TimeSeries analytical;
  CalculateAnalyticalSolution(
      &analytical, beta, gamma, sparam->initial_population_susceptible,
      sparam->initial_population_infected, 0, sparam->number_of_iterations, 1);
  return analytical;
}

void ExperimentSimAndAnalytical(CommandLineOptions* clo,
                                const std::vector<double>& seeds) {
  auto analytical = GetAnalyticalResults(clo);
  double mse = Experiment(clo, seeds, analytical, true);
  std::cout << " MSE " << mse << std::endl;
}

void ExperimentFitSimulation(CommandLineOptions* clo,
                             const std::vector<double>& seeds) {
  auto analytical = GetAnalyticalResults(clo);

  auto fit = [&](const arma::vec& inout, arma::vec* grad_out, void* opt_data) {
    double mse = Experiment(clo, seeds, analytical, false, true, inout(0),
                            inout(1), inout(2));
    std::cout << " MSE " << mse << " inout " << inout(0) << " - " << inout(1)
              << " - " << inout(2) << std::endl
              << std::endl;
    return mse;
  };

  // Create simulation object just to obtain parameter values
  Simulation sim(clo, [](Param* param) { param->statistics = false; });
  auto* param = sim.GetParam();
  auto* sparam = param->Get<SimParam>();

  // setup optimization algorithm
  arma::vec inout({sparam->infection_probablity, sparam->infection_radius,
                   sparam->agent_speed});
  optim::algo_settings_t settings;
  settings.vals_bound = true;
  settings.lower_bounds = arma::vec({0.001, 5, 2});
  settings.upper_bounds =
      arma::vec({1, param->max_bound / 2, param->max_bound / 2});
  if (!optim::pso(inout, fit, nullptr, settings)) {
    Log::Fatal("", "Optimization algorithm didn't complete successfully.");
  }

  // final result
  double mse = Experiment(clo, seeds, analytical, true, true, inout(0),
                          inout(1), inout(2));
  std::cout << "Final MSE " << mse << std::endl;
}

}  // namespace bdm

int main(int argc, const char** argv) {
  // register parameters that are specific for this simulation
  bdm::Param::RegisterParamGroup(new bdm::SimParam());

  // define additional command line options
  bdm::CommandLineOptions clo(argc, argv);
  clo.AddOption<std::string>("mode", "sim-and-analytical");
  clo.AddOption<double>("beta", "0.06719");
  clo.AddOption<double>("gamma", "0.00521");
  clo.AddOption<uint64_t>("repeat", "10");
  clo.AddOption<bool>("no-legend", "false");
  auto mode = clo.Get<std::string>("mode");

  std::vector<double> seeds;
  bdm::Random r;
  r.SetSeed(2444);
  for (uint64_t i = 0; i < clo.Get<uint64_t>("repeat"); ++i) {
    seeds.push_back(r.Uniform(0, std::numeric_limits<uint16_t>::max()));
  }

  for (auto el : seeds) {
    std::cout << el << std::endl;
  }

  if (mode == "sim-and-analytical") {
    bdm::ExperimentSimAndAnalytical(&clo, seeds);
  } else if (mode == "fit-simulation") {
    bdm::ExperimentFitSimulation(&clo, seeds);
  }

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}
