// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef EPIDEMIOLOGY_H_
#define EPIDEMIOLOGY_H_

#include "biodynamo.h"
#include "core/environment/uniform_grid_environment.h"

#include "analytical-solution.h"
#include "behavior.h"
#include "evaluate.h"
#include "person.h"
#include "sim-param.h"

namespace bdm {

// This is the main simulation function
inline int Simulate(int argc, const char** argv, TimeSeries* result,
                    Param* final_params = nullptr) {
  // Overwrite the parameters with the `final_params` we obtain from the obtain
  // from `bdm::Experiment`
  auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
    param->random_seed = 0;
    param->simulation_time_step = 1;
    param->bound_space = Param::BoundSpaceMode::kTorus;
  };

  // Create simulation object
  Simulation sim(argc, argv, set_param);

  // Get pointers to important objects
  auto* param = sim.GetParam();
  auto* sparam = param->Get<SimParam>();
  auto* random = sim.GetRandom();
  auto* env = dynamic_cast<UniformGridEnvironment*>(sim.GetEnvironment());

  auto state = State::kSusceptible;
  // Lambda that creates a new person at specific position in space
  auto person_creator = [&](const Double3& position) {
    auto* person = new Person(position);
    // Set the data members of the new person
    person->SetDiameter(sparam->agent_diameter);
    person->state_ = state;
    // Define the persons behavior
    person->AddBehavior(new Infection());
    person->AddBehavior(new Recovery());
    if (random->Uniform() < sparam->moving_agents_ratio) {
      person->AddBehavior(new RandomMovement());
    }
    return person;
  };

  // Create an initial population of susceptible persons.
  // Person will be randomly distributed.
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound,
                                       sparam->initial_population_susceptible,
                                       person_creator);

  // Create an initial population of infected persons.
  // Person will be randomly distributed.
  state = State::kInfected;
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound,
                                       sparam->initial_population_infected,
                                       person_creator);

  SetupResultCollection(&sim);

  // Set the box length to the infection radius
  env->SetBoxLength(sparam->infection_radius);

  // Now we finished defining the initial simulation state.

  auto* scheduler = sim.GetScheduler();
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  // Simulate for SimParam::number_of_iterations steps
  {
    Timing timer("RUNTIME");
    scheduler->Simulate(sparam->number_of_iterations);
  }
  // move time series data from simulation to result
  *result = std::move(*sim.GetTimeSeries());

  return 0;
}

}  // namespace bdm

#endif  // EPIDEMIOLOGY_H_
