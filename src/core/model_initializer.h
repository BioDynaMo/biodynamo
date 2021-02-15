// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_MODEL_INITIALIZER_H_
#define CORE_MODEL_INITIALIZER_H_

#include <ctime>
#include <string>
#include <vector>

#include "core/container/math_array.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/random.h"

class EulerGrid;
class StencilGrid;
class RungaKuttaGrid;

namespace bdm {

struct ModelInitializer {
  /// Creates a 3D cubic grid of agents and adds them to the
  /// ExecutionContext. Type of the agent is determined by the return
  /// type of parameter agent_builder.
  ///
  ///     ModelInitializer::Grid3D(8, 10, [](const Double3& pos){
  ///     return Cell(pos); });
  /// @param      agents_per_dim  number of agents on each axis.
  ///                            Number of generated agents =
  ///                            `agents_per_dim ^ 3`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      agent_builder   function containing the logic to instantiate a
  ///                            new agent. Takes `const
  ///                            Double3&` as input parameter
  ///
  template <typename Function>
  static void Grid3D(size_t agents_per_dim, double space,
                     Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();

#pragma omp for
      for (size_t x = 0; x < agents_per_dim; x++) {
        auto x_pos = x * space;
        for (size_t y = 0; y < agents_per_dim; y++) {
          auto y_pos = y * space;
          for (size_t z = 0; z < agents_per_dim; z++) {
            auto* new_agent = agent_builder({x_pos, y_pos, z * space});
            ctxt->AddAgent(new_agent);
          }
        }
      }
    }
  }

  /// Creates a 3D grid of agents and adds them to the
  /// ExecutionContext. Type of the agent is determined by the return
  /// type of parameter agent_builder.
  ///
  ///     ModelInitializer::Grid3D({8,6,4}, 10, [](const Double3&
  ///     pos){ return Cell(pos); });
  /// @param      agents_per_dim  number of agents on each axis.
  ///                            Number of generated agents =
  ///                            `agents_per_dim[0] * agents_per_dim[1] *
  ///                            agents_per_dim[2]`
  /// @param      space          space between the positions - e.g space = 10:
  ///                            positions = `{(0, 0, 0), (0, 0, 10), (0, 0,
  ///                            20), ... }`
  /// @param      agent_builder   function containing the logic to instantiate a
  ///                            new agent. Takes `const
  ///                            Double3&` as input parameter
  ///
  template <typename Function>
  static void Grid3D(const std::array<size_t, 3>& agents_per_dim, double space,
                     Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();

#pragma omp for
      for (size_t x = 0; x < agents_per_dim[0]; x++) {
        auto x_pos = x * space;
        for (size_t y = 0; y < agents_per_dim[1]; y++) {
          auto y_pos = y * space;
          for (size_t z = 0; z < agents_per_dim[2]; z++) {
            auto* new_agent = agent_builder({x_pos, y_pos, z * space});
            ctxt->AddAgent(new_agent);
          }
        }
      }
    }
  }

  /// Adds agents to the ExecutionContext. Type of the simulation
  /// object is determined by the return type of parameter agent_builder.
  ///
  /// @param      positions     positions of the agents to be
  /// @param      agent_builder  function containing the logic to instantiate a
  ///                           new agent. Takes `const
  ///                           Double3&` as input parameter
  ///
  template <typename Function>
  static void CreateAgents(const std::vector<Double3>& positions,
                           Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();

#pragma omp for
      for (size_t i = 0; i < positions.size(); i++) {
        auto* new_agent =
            agent_builder({positions[i][0], positions[i][1], positions[i][2]});
        ctxt->AddAgent(new_agent);
      }
    }
  }

  /// Adds agents with random positions to the ExecutionContext.
  /// Type of the agent is determined by the return type of
  /// parameter agent_builder.
  ///
  /// @param[in]  min           The minimum position value
  /// @param[in]  max           The maximum position value
  /// @param[in]  num_agents     The number agents
  /// @param[in]  agent_builder  function containing the logic to instantiate a
  ///                           new agent. Takes `const
  ///                           Double3&` as input parameter
  ///
  template <typename Function>
  static void CreateAgentsRandom(double min, double max, uint64_t num_agents,
                                 Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();
      auto* random = sim->GetRandom();

#pragma omp for
      for (uint64_t i = 0; i < num_agents; i++) {
        auto* new_agent = agent_builder(random->UniformArray<3>(min, max));
        ctxt->AddAgent(new_agent);
      }
    }
  }

  /// Allows agents to secrete the specified substance. Diffusion throughout the
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
                              int resolution = 10);

  template <typename F>
  static void InitializeSubstance(size_t substance_id, F function) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto diffusion_grid = rm->GetDiffusionGrid(substance_id);
    diffusion_grid->AddInitializer(function);
  }
};

}  // namespace bdm

#endif  // CORE_MODEL_INITIALIZER_H_
