// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef INTEGRATION_SOMA_CLUSTERING_H_
#define INTEGRATION_SOMA_CLUSTERING_H_

#include <vector>

#include "biodynamo.h"
#include "math_util.h"

namespace bdm {

// -----------------------------------------------------------------------------
// This model examplifies the use of extracellur diffusion and shows
// how to extend the default "Cell". In step 0 one can see how an extra
// data member is added and can be accessed throughout the simulation with
// its Get and Set methods. N cells are randomly positioned in space, of which
// half are of type 1 and half of type -1.
//
// Each type secretes a different substance. Cells move towards the gradient of
// their own substance, which results in clusters being formed of cells of the
// same type.
// -----------------------------------------------------------------------------

// 0. Define my custom cell, which extends Cell by adding an extra
// data member cell_type.
BDM_SIM_OBJECT(MyCell, bdm::Cell) {
  BDM_SIM_OBJECT_HEADER(MyCellExt, 1, cell_type_);

 public:
  MyCellExt() {}
  // TODO(ahmad): this needs to be explicitely stated, otherwise empty
  // implementation
  MyCellExt(const std::array<double, 3>& position) : Base(position) {}

  void SetCellType(int t) { cell_type_[kIdx] = t; }
  int GetCellType() const { return cell_type_[kIdx]; }

 private:
  vec<int> cell_type_;
};

enum Substances { kSubstance_0, kSubstance_1 };

// 1a. Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  // Daughter cells inherit this biology module
  Chemotaxis() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    if (!init_) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      dg_0_ = rm->GetDiffusionGrid(kSubstance_0);
      dg_1_ = rm->GetDiffusionGrid(kSubstance_1);
      init_ = true;
    }

    auto& position = cell->GetPosition();
    std::array<double, 3> diff_gradient;

    if (cell->GetCellType() == 1) {
      dg_1_->GetGradient(position, &gradient_1_);
      diff_gradient = Math::ScalarMult(5, gradient_1_);
    } else {
      dg_0_->GetGradient(position, &gradient_0_);
      diff_gradient = Math::ScalarMult(5, gradient_0_);
    }

    cell->UpdatePosition(diff_gradient);
  }

 private:
  bool init_ = false;
  DiffusionGrid* dg_0_ = nullptr;
  DiffusionGrid* dg_1_ = nullptr;
  std::array<double, 3> gradient_0_;
  std::array<double, 3> gradient_1_;
  ClassDefNV(Chemotaxis, 1);
};

// 1b. Define secretion behavior:
struct SubstanceSecretion : public BaseBiologyModule {
  // Daughter cells inherit this biology module
  SubstanceSecretion() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    if (!init_) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      dg_0_ = rm->GetDiffusionGrid(kSubstance_0);
      dg_1_ = rm->GetDiffusionGrid(kSubstance_1);
      init_ = true;
    }
    auto& secretion_position = cell->GetPosition();
    if (cell->GetCellType() == 1) {
      dg_1_->IncreaseConcentrationBy(secretion_position,
                                     1 / dg_1_->GetBoxVolume());
    } else {
      dg_0_->IncreaseConcentrationBy(secretion_position,
                                     1 / dg_0_->GetBoxVolume());
    }
  }

 private:
  bool init_ = false;
  DiffusionGrid* dg_0_ = nullptr;
  DiffusionGrid* dg_1_ = nullptr;
  ClassDefNV(SubstanceSecretion, 1);
};

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<Chemotaxis, SubstanceSecretion>;
  using AtomicTypes = VariadicTypedef<MyCell>;
};

// Returns 0 if the cell locations within a subvolume of the total system,
// comprising approximately target_n cells, are arranged as clusters, and 1
// otherwise.
template <typename TSimulation = Simulation<>>
static bool GetCriterion(double spatial_range, int target_n) {
  auto* sim = TSimulation::GetActive();
  auto* rm = sim->GetResourceManager();

  auto my_cells = rm->template Get<MyCell>();
  int n = my_cells->size();

  // number of cells that are close (i.e. within a distance of
  // spatial_range)
  int num_close = 0;
  double curr_dist;
  // number of cells of the same type, and that are close (i.e.
  // within a distance of spatial_range)
  int same_type_close = 0;
  // number of cells of opposite types, and that are close (i.e.
  // within a distance of spatial_range)
  int diff_type_close = 0;

  std::vector<array<double, 3>> pos_sub_vol(n);
  std::vector<int> types_sub_vol(n);

  // Define the subvolume to be the first octant of a cube
  auto* param = sim->GetParam();
  double sub_vol_max = param->max_bound_ / 2;

  // The number of cells within the subvolume
  int num_cells_sub_vol = 0;

  // the locations of all cells within the subvolume are copied
  // to pos_sub_vol
  for (int i1 = 0; i1 < n; i1++) {
    auto& pos = (*my_cells)[i1].GetPosition();
    auto type = (*my_cells)[i1].GetCellType();

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

  std::cout << "number of cells in subvolume: " << num_cells_sub_vol
            << std::endl;

  // If there are not enough cells within the subvolume, the correctness
  // criterion is not fulfilled
  if (((static_cast<double>((num_cells_sub_vol))) /
       static_cast<double>(target_n)) < 0.25) {
    std::cout << "not enough cells in subvolume: " << num_cells_sub_vol
              << std::endl;
    return false;
  }

  // If there are too many cells within the subvolume, the correctness
  // criterion is not fulfilled
  if (((static_cast<double>((num_cells_sub_vol))) /
       static_cast<double>(target_n)) > 4) {
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

  double correctness_coefficient =
      (static_cast<double>(diff_type_close)) / (num_close + 1.0);

  // check if there are many cells of opposite types located within a close
  // distance, indicative of bad clustering
  if (correctness_coefficient > 0.1) {
    std::cout << "cells in subvolume are not well-clustered: "
              << correctness_coefficient << std::endl;
    return false;
  }

  // check if clusters are large enough, i.e. whether cells have more than 100
  // cells of the same type located nearby
  double avg_neighbors =
      (static_cast<double>(same_type_close / num_cells_sub_vol));
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

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);
  auto* param = simulation.GetParam();

  // 3. Define initial model

  // Create an artificial bounds for the simulation space
  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 250;
  param->run_mechanical_interactions_ = false;
  int num_cells = 20000;

// set seed for each thread local random number generator
#pragma omp parallel
  simulation.GetRandom()->SetSeed(4357);

  // Construct num_cells/2 cells of type 1
  auto construct_0 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(1);
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_,
                                      num_cells / 2, construct_0);

  // Construct num_cells/2 cells of type -1
  auto construct_1 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(-1);
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_,
                                      num_cells / 2, construct_1);

  // 3. Define the substances that cells may secrete
  // Order: substance_name, diffusion_coefficient, decay_constant, resolution
  ModelInitializer::DefineSubstance(kSubstance_0, "Substance_0", 0.5, 0.1, 25);
  ModelInitializer::DefineSubstance(kSubstance_1, "Substance_1", 0.5, 0.1, 25);

  // 4. Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(3001);

  double spatial_range = 5;
  auto crit = GetCriterion(spatial_range, num_cells / 8);
  return !crit;
}

}  // namespace bdm

#endif  // INTEGRATION_SOMA_CLUSTERING_H_
