// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef EPIDEMIOLOGY_H_
#define EPIDEMIOLOGY_H_

#include "biodynamo.h"

#include "analytical-solution.h"
#include "behavior.h"
#include "evaluate.h"
#include "person.h"
#include "sim-param.h"

namespace bdm {

// This is the main simulation function
inline int Simulate(CommandLineOptions* clo, double seed, ResultData* result,
                    bool overwrite = false, double infection_probablity = 1,
                    double infection_radius = 1, double speed = 1) {
  // Overwrite the parameters in the config file with the value from
  // the command line options
  auto set_param = [&](Param* param) {
    param->random_seed = seed;
    param->simulation_time_step = 1;
    param->bound_space = true;
    if (overwrite) {
      auto* sparam = param->Get<SimParam>();
      sparam->infection_probablity = infection_probablity;
      sparam->infection_radius = infection_radius;
      sparam->agent_speed = speed;
    }
  };

  // Create simulation object
  Simulation sim(clo, set_param);

  // Get pointers to important objects
  auto* param = sim.GetParam();
  auto* sparam = param->Get<SimParam>();
  auto* random = sim.GetRandom();

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

  // workaround for bug -> ignore
  auto* cell = new Person();
  cell->SetDiameter(sparam->infection_radius);
  sim.GetResourceManager()->AddAgent(cell);
  // end of workaround

  // Now we finished defining the initial simulation state.

  // Setup counting operation
  auto* count_op = NewOperation("ReductionOpDouble4");
  auto* count_op_impl = count_op->GetImplementation<ReductionOp<Double4>>();
  count_op_impl->Initialize(new CountSIR(), new CalcRates());
  auto* scheduler = sim.GetScheduler();
  scheduler->ScheduleOp(count_op);

  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  // Simulate for SimParam::number_of_iterations steps
  {
    Timing timer("RUNTIME");
    scheduler->Simulate(sparam->number_of_iterations);
  }
  TransferResult(result, count_op_impl->GetResults());

  return 0;
}

}  // namespace bdm

#endif  // EPIDEMIOLOGY_H_
