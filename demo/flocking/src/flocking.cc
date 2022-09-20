// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// Author: Moritz Grabmann (2022)
//
// -----------------------------------------------------------------------------

#include "flocking.h"
#include "biodynamo.h"
#include "boid.h"
#include "sim_param.h"
#include "update_operation.h"

namespace bdm {

const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

int Simulate(int argc, const char** argv) {
  // ---------------------------------------------------------------------------
  // Set references
  Param::RegisterParamGroup(new SimParam());
  Simulation simulation(argc, argv);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  auto* sparam = param->Get<SimParam>();
  auto* scheduler = simulation.GetScheduler();

  // ---------------------------------------------------------------------------
  // Spawn and initialize boids
  real_t centre = (param->max_bound + param->min_bound) / 2;
  real_t radius = sparam->starting_sphere_radius;

  Real3 transl = {centre, centre, centre};

  for (size_t i = 0; i < sparam->n_boids; ++i) {
    auto* boid = new Boid();
    Real3 coord = GetRandomVectorInUnitSphere() * radius + transl;

    boid->SetPosition(coord);
    boid->SetVelocity({0, 0, 0});
    boid->SetHeadingDirection(GetRandomVectorInUnitSphere());
    boid->AddBehavior(new Flocking());
    boid->AddBehavior(
        new RandomPerturbation(sparam->random_perturbation_strength));
    boid->InitializeMembers();

    rm->AddAgent(boid);
  }

  // ---------------------------------------------------------------------------
  // Add a PostScheduledOp to update the boids position and velocity after each
  // computational step
  OperationRegistry::GetInstance()->AddOperationImpl(
      "UpdateOp", OpComputeTarget::kCpu, new UpdateOp());
  auto* update_op = NewOperation("UpdateOp");
  scheduler->ScheduleOp(update_op, OpType::kPostSchedule);

  // ---------------------------------------------------------------------------
  // Simulate
  scheduler->Simulate(sparam->computational_steps);

  // ---------------------------------------------------------------------------
  // End of simulation
  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
};

}  // namespace bdm
