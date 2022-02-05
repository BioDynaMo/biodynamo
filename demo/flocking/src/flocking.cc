// -----------------------------------------------------------------------------
//
// Copyright (C) Moritz Grabmann.
// All Rights Reserved.
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
  auto set_param = [&](Param* param) {};
  Simulation simulation(argc, argv, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  auto* sparam = param->Get<SimParam>();
  auto* scheduler = simulation.GetScheduler();

  // ---------------------------------------------------------------------------
  // Spawn and initialize boids
  double centre = (param->max_bound + param->min_bound) / 2;
  double radius = sparam->starting_sphere_radius;

  Double3 transl = {centre, centre, centre};

  for (size_t i = 0; i < sparam->n_boids; ++i) {
    auto* boid = new Boid();
    Double3 coord = GetRandomVectorInUnitSphere() * radius + transl;

    boid->SetPosition(coord);
    boid->SetVelocity({0, 0, 0});
    boid->SetHeadingDirection(GetRandomVectorInUnitSphere());
    boid->AddBehavior(new Flocking());
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
