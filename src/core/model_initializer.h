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

#ifndef CORE_MODEL_INITIALIZER_H_
#define CORE_MODEL_INITIALIZER_H_

#include <ctime>
#include <string>
#include <vector>

#include "core/container/math_array.h"
#include "core/diffusion_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/random.h"

namespace bdm {

struct ModelInitializer {
  /// Creates a 3D cubic grid of simulation objects and adds them to the
  /// ResourceManager. Type of the simulation object is determined by the return
  /// type of parameter cell_builder.
  ///
  ///     ModelInitializer::Grid3D(8, 10, [](const Double3& pos){
  ///     return Cell(pos); });
  /// @param      cells_per_dim  number of simulation objects on each axis.
  ///                            Number of generated simulation objects =
  ///                            `cells_per_dim ^ 3`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      cell_builder   function containing the logic to instantiate a
  ///                            new simulation object. Takes `const
  ///                            Double3&` as input parameter
  ///
  template <typename Function>
  static void Grid3D(size_t cells_per_dim, double space,
                     Function cell_builder) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    rm->Reserve(cells_per_dim * cells_per_dim * cells_per_dim);
    for (size_t x = 0; x < cells_per_dim; x++) {
      auto x_pos = x * space;
      for (size_t y = 0; y < cells_per_dim; y++) {
        auto y_pos = y * space;
        for (size_t z = 0; z < cells_per_dim; z++) {
          auto* new_simulation_object = cell_builder({x_pos, y_pos, z * space});
          rm->push_back(new_simulation_object);
        }
      }
    }
  }

  /// Creates a 3D grid of simulation objects and adds them to the
  /// ResourceManager. Type of the simulation object is determined by the return
  /// type of parameter cell_builder.
  ///
  ///     ModelInitializer::Grid3D({8,6,4}, 10, [](const Double3&
  ///     pos){ return Cell(pos); });
  /// @param      cells_per_dim  number of simulation objects on each axis.
  ///                            Number of generated simulation objects =
  ///                            `cells_per_dim[0] * cells_per_dim[1] *
  ///                            cells_per_dim[2]`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      cell_builder   function containing the logic to instantiate a
  ///                            new simulation object. Takes `const
  ///                            Double3&` as input parameter
  ///
  template <typename Function>
  static void Grid3D(const std::array<size_t, 3>& cells_per_dim, double space,
                     Function cell_builder) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    rm->Reserve(cells_per_dim[0] * cells_per_dim[1] * cells_per_dim[2]);
    for (size_t x = 0; x < cells_per_dim[0]; x++) {
      auto x_pos = x * space;
      for (size_t y = 0; y < cells_per_dim[1]; y++) {
        auto y_pos = y * space;
        for (size_t z = 0; z < cells_per_dim[2]; z++) {
          auto* new_simulation_object = cell_builder({x_pos, y_pos, z * space});
          rm->push_back(new_simulation_object);
        }
      }
    }
  }

  /// Adds simulation objects to the ResourceManager. Type of the simulation
  /// object is determined by the return type of parameter cell_builder.
  ///
  /// @param      positions     positions of the simulation objects to be
  /// @param      cell_builder  function containing the logic to instantiate a
  ///                           new simulation object. Takes `const
  ///                           Double3&` as input parameter
  ///
  template <typename Function>
  static void CreateCells(const std::vector<Double3>& positions,
                          Function cell_builder) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    rm->Reserve(positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
      auto* new_simulation_object =
          cell_builder({positions[i][0], positions[i][1], positions[i][2]});
      rm->push_back(new_simulation_object);
    }
  }

  /// Adds simulation objects with random positions to the ResourceManager.
  /// Type of the simulation object is determined by the return type of
  /// parameter cell_builder.
  ///
  /// @param[in]  min           The minimum position value
  /// @param[in]  max           The maximum position value
  /// @param[in]  num_cells     The number cells
  /// @param[in]  cell_builder  function containing the logic to instantiate a
  ///                           new simulation object. Takes `const
  ///                           Double3&` as input parameter
  ///
  template <typename Function>
  static void CreateCellsRandom(double min, double max, int num_cells,
                                Function cell_builder) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    rm->Reserve(num_cells);

    // TODO(ahmad): throughout simulation only one random number generator
    // should be used, so this should go someplace accessible for other
    // classes / functions
    auto* random = sim->GetRandom();
    for (int i = 0; i < num_cells; i++) {
      double x = random->Uniform(min, max);
      double y = random->Uniform(min, max);
      double z = random->Uniform(min, max);
      auto* new_simulation_object = cell_builder({x, y, z});
      rm->push_back(new_simulation_object);
    }
  }

  /// Allows cells to secrete the specified substance. Diffusion throughout the
  /// simulation space is automatically taken care of by the DiffusionGrid class
  ///
  /// @param[in]  substance_id     The substance identifier
  /// @param[in]  substance_name   The substance name
  /// @param[in]  diffusion_coeff  The diffusion coefficient
  /// @param[in]  decay_constant   The decay constant
  /// @param[in]  resolution       The resolution of the diffusion grid
  ///
  static void DefineSubstance(size_t substance_id, std::string substance_name,
                              double diffusion_coeff, double decay_constant,
                              int resolution = 10, unsigned int diffusion_step = 1,
                              std::string boundary = "Open") {
    assert(resolution > 0 && "Resolution needs to be a positive integer value");
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto* param = sim->GetParam();

    if (param->diffusion_type_ == "RK") {
       RKGrid* d_grid =
        new RKGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution, diffusion_step, boundary);
        rm->AddDiffusionGrid(d_grid);
    } else {
       EulerGrid* d_grid =
        new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution, diffusion_step, boundary);
        rm->AddDiffusionGrid(d_grid);
    }
  }

  template <typename F>
  static void InitializeSubstance(size_t substance_id,
                                  std::string substance_name, F function) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto diffusion_grid = rm->GetDiffusionGrid(substance_id);
    diffusion_grid->AddInitializer(function);
  }
};

}  // namespace bdm

#endif  // CORE_MODEL_INITIALIZER_H_
