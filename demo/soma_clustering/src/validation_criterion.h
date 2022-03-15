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

#ifndef VALIDATION_CRITERION_H_
#define VALIDATION_CRITERION_H_

#include <vector>
#include "biodynamo.h"
#include "my_cell.h"

namespace bdm {
namespace soma_clustering {

// Returns 0 if the cell locations within a subvolume of the total system,
// comprising approximately target_n cells, are arranged as clusters, and 1
// otherwise.
static bool GetCriterion(real spatial_range, int target_n) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* param = sim->GetParam();

  // get number of MyCells
  int n = rm->GetNumAgents();

  // number of cells that are close (i.e. within a distance of
  // spatial_range)
  int num_close = 0;
  real curr_dist;
  // number of cells of the same type, and that are close (i.e.
  // within a distance of spatial_range)
  int same_type_close = 0;
  // number of cells of opposite types, and that are close (i.e.
  // within a distance of spatial_range)
  int diff_type_close = 0;

  std::vector<Real3> pos_sub_vol(n);
  std::vector<int> types_sub_vol(n);

  // Define the subvolume to be the first octant of a cube
  real sub_vol_max = param->max_bound / 2;

  // The number of cells within the subvolume
  int num_cells_sub_vol = 0;

  // the locations of all cells within the subvolume are copied
  // to pos_sub_vol
  rm->ForEachAgent([&](Agent* agent) {
    if (auto* cell = dynamic_cast<MyCell*>(agent)) {
      const auto& pos = cell->GetPosition();
      auto type = cell->GetCellType();

      if ((fabs(pos[0] - 0.5) < sub_vol_max) &&
          (fabs(pos[1] - 0.5) < sub_vol_max) &&
          (fabs(pos[2] - 0.5) < sub_vol_max)) {
        pos_sub_vol[num_cells_sub_vol][0] = pos[0];
        pos_sub_vol[num_cells_sub_vol][1] = pos[1];
        pos_sub_vol[num_cells_sub_vol][2] = pos[2];
        types_sub_vol[num_cells_sub_vol] = type;
        num_cells_sub_vol++;
      }
    }
  });

  std::cout << "number of cells in subvolume: " << num_cells_sub_vol
            << std::endl;

  // If there are not enough cells within the subvolume, the correctness
  // criterion is not fulfilled
  if (((static_cast<real>((num_cells_sub_vol))) /
       static_cast<real>(target_n)) < 0.25) {
    std::cout << "not enough cells in subvolume: " << num_cells_sub_vol
              << std::endl;
    return false;
  }

  // If there are too many cells within the subvolume, the correctness
  // criterion is not fulfilled
  if (((static_cast<real>((num_cells_sub_vol))) /
       static_cast<real>(target_n)) > 4) {
    std::cout << "too many cells in subvolume: " << num_cells_sub_vol
              << std::endl;
    return false;
  }

#pragma omp parallel for reduction(+ : same_type_close, diff_type_close, \
                                   num_close)
  for (int i1 = 0; i1 < num_cells_sub_vol; i1++) {
    for (int i2 = i1 + 1; i2 < num_cells_sub_vol; i2++) {
      curr_dist = Math::GetL2Distance(pos_sub_vol[i1], pos_sub_vol[i2]);
      if (curr_dist < spatial_range) {
        num_close++;
        if (types_sub_vol[i1] * types_sub_vol[i2] < 0) {
          diff_type_close++;
        } else {
          same_type_close++;
        }
      }
    }
  }

  real correctness_coefficient =
      (static_cast<real>(diff_type_close)) / (num_close + 1.0);

  // check if there are many cells of opposite types located within a close
  // distance, indicative of bad clustering
  if (correctness_coefficient > 0.1) {
    std::cout << "cells in subvolume are not well-clustered: "
              << correctness_coefficient << std::endl;
    return false;
  }

  // check if clusters are large enough, i.e. whether cells have more than 100
  // cells of the same type located nearby
  real avg_neighbors =
      (static_cast<real>(same_type_close / num_cells_sub_vol));
  std::cout << "average neighbors in subvolume: " << avg_neighbors << std::endl;
  if (avg_neighbors < 5) {
    std::cout << "cells in subvolume do not have enough neighbors: "
              << avg_neighbors << std::endl;
    return false;
  }

  std::cout << "correctness coefficient: " << correctness_coefficient
            << std::endl;

  return true;
}

}  // namespace soma_clustering
}  // namespace bdm

#endif  // VALIDATION_CRITERION_H_
