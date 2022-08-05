// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <mpi.h>
#include "core/distribution/distribution_param.h"
#include "core/simulation.h"
#include "core/simulation_space.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(SpatialSTKDistributor, Initialize) {
  auto set_param = [](Param* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -10;
    param->max_bound = 20;
    param->interaction_radius = 5;
    param->Get<experimental::DistributionParam>()->box_length_factor = 2;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* space = simulation.GetSimulationSpace();

  SimulationSpace::Space expected_ws = {-10, 20, -10, 20, -10, 20};
  EXPECT_EQ(expected_ws, space->GetWholeSpace());

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    SimulationSpace::Space expected_ls = {-10, 20, -10, 20, -10, 10};
    EXPECT_EQ(expected_ls, space->GetLocalSpace());
  } else {
    SimulationSpace::Space expected_ls = {-10, 20, -10, 20, 10, 20};
    EXPECT_EQ(expected_ls, space->GetLocalSpace());
  }
}

}  // namespace bdm
