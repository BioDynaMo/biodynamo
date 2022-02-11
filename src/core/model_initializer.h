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

#ifndef CORE_MODEL_INITIALIZER_H_
#define CORE_MODEL_INITIALIZER_H_

#include <Math/DistFunc.h>
#include <ctime>
#include <string>
#include <vector>

#include "core/container/math_array.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/random.h"

class EulerGrid;
class RungeKuttaGrid;

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

  /// Creates agents on the given positions and adds them to the
  /// ExecutionContext.
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

  /// Creates agents with random positions and adds them to the
  /// ExecutionContext. Agent creation is parallelized.
  ///
  /// @param[in]  min           The minimum position value
  /// @param[in]  max           The maximum position value
  /// @param[in]  num_agents     The number agents
  /// @param[in]  agent_builder  function containing the logic to instantiate a
  ///                           new agent. Takes `const
  ///                           Double3&` as input parameter
  /// \param[in]  rng           Uses the given DistributionRng.
  ///                           if rng is a nullptr, this function uses a
  ///                           uniform distribution between [min, max[
  template <typename Function>
  static void CreateAgentsRandom(double min, double max, uint64_t num_agents,
                                 Function agent_builder,
                                 DistributionRng<double>* rng = nullptr) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();
      auto* random = sim->GetRandom();

#pragma omp for
      for (uint64_t i = 0; i < num_agents; i++) {
        if (rng != nullptr) {
          Double3 pos;
          bool in_range = false;
          do {
            pos = rng->Sample3();
            in_range = (pos[0] >= min) && (pos[0] <= max) && (pos[1] >= min) &&
                       (pos[1] <= max) && (pos[2] >= min) && (pos[2] <= max);
          } while (!in_range);
          auto* new_agent = agent_builder(pos);
          ctxt->AddAgent(new_agent);
        } else {
          auto* new_agent = agent_builder(random->UniformArray<3>(min, max));
          ctxt->AddAgent(new_agent);
        }
      }
    }
  }

  /// Creates agents on surface and adds them to the ExecutionContext.
  /// The x and y positions are defined by xmin, xmax, deltax and ymin, ymax,
  /// deltay. The z position is calculated using `f`. Agent creation is
  /// parallelized.
  ///
  ///     auto construct = [](const Double3& position) {
  ///       Cell* cell = new Cell(position);
  ///       cell->SetDiameter(10);
  ///       return cell;
  ///     };
  ///     auto f = [](const double* x, const double* params) {
  ///         // 10 * sin(x/20) + 10 * sin(y/20)
  ///         return 10 * std::sin(x[0] / 20.) + 10 * std::sin(x[1] / 20.0);
  ///     };
  ///     ModelInitializer::CreateAgentsOnSurface(f, {}, -100, 100, 10, -100,
  ///     100, 10, construct);
  ///
  /// \param[in]  f             function that defines the surface
  /// \param[in]  fn_params     Parameters that will be passed to `f` as
  ///                           second argument.
  /// @param[in]  xmin          Minimum x coordinate on which a agent will be
  /// created.
  /// @param[in]  xmax          Maximum x coordinate on which a agent will be
  /// created.
  /// @param[in]  deltax        Space between two agents on the x-axis.
  /// @param[in]  ymin          Minimum y coordinate on which a agent will be
  /// created.
  /// @param[in]  ymax          Maximum y coordinate on which a agent will be
  /// created.
  /// @param[in]  deltay        Space between two agents on the y-axis.
  /// @param[in]  agent_builder function containing the logic to instantiate a
  ///                           new agent. Takes `const Double3&` as input
  ///                           parameter
  template <typename Function>
  static void CreateAgentsOnSurface(
      double (*f)(const double*, const double*),
      const FixedSizeVector<double, 10>& fn_params, double xmin, double xmax,
      double deltax, double ymin, double ymax, double deltay,
      Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();

      auto xiterations =
          static_cast<uint64_t>(std::floor((xmax - xmin) / deltax));
      auto yiterations =
          static_cast<uint64_t>(std::floor((ymax - ymin) / deltay));

#pragma omp for
      for (uint64_t xit = 0; xit < xiterations; ++xit) {
        double x = xmin + xit * deltax;
        for (uint64_t yit = 0; yit < yiterations; ++yit) {
          double y = ymin + yit * deltay;
          Double3 pos = {x, y};
          pos[2] = f(pos.data(), fn_params.data());
          ctxt->AddAgent(agent_builder(pos));
        }
      }
    }
  }

  /// Creates agents on surface and adds them to the ExecutionContext.
  /// The x and y positions are determined by a uniform distribution [xmin,
  /// xmax[ and [ymin, ymax[. The z position is calculated using `f`. Agent
  /// creation is parallelized.
  ///
  ///     auto construct = [](const Double3& position) {
  ///       Cell* cell = new Cell(position);
  ///       cell->SetDiameter(10);
  ///       return cell;
  ///     };
  ///     auto f = [](const double* x, const double* params) {
  ///         // 10 * sin(x/20) + 10 * sin(y/20)
  ///         return 10 * std::sin(x[0] / 20.) + 10 * std::sin(x[1] / 20.0);
  ///     };
  ///     ModelInitializer::CreateAgentsOnSurfaceRndm(f, {}, -100, 100, -100,
  ///     100, construct);
  ///
  /// \param[in]  f             function that defines the surface
  /// \param[in]  fn_params     Parameters that will be passed to `f` as
  ///                           second argument.
  /// @param[in]  xmin          Minimum x coordinate on which a agent will be
  /// created.
  /// @param[in]  xmax          Maximum x coordinate on which a agent will be
  /// created.
  /// @param[in]  ymin          Minimum y coordinate on which a agent will be
  /// created.
  /// @param[in]  ymax          Maximum y coordinate on which a agent will be
  /// created.
  /// @param[in]  agent_builder function containing the logic to instantiate a
  ///                           new agent. Takes `const Double3&` as input
  ///                           parameter
  template <typename Function>
  static void CreateAgentsOnSurfaceRndm(
      double (*f)(const double*, const double*),
      const FixedSizeVector<double, 10>& fn_params, double xmin, double xmax,
      double ymin, double ymax, uint64_t num_agents, Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();
      auto* random = sim->GetRandom();

#pragma omp for
      for (uint64_t i = 0; i < num_agents; ++i) {
        Double3 pos = {random->Uniform(xmin, xmax),
                       random->Uniform(ymin, ymax)};
        pos[2] = f(pos.data(), fn_params.data());
        ctxt->AddAgent(agent_builder(pos));
      }
    }
  }

  /// Creates agents with random positions on a sphere and adds them to the
  /// ExecutionContext. The agents' positions are uniformly distributed accross
  /// the surface. Agent creation is parallelized.
  /// Algorithm: Knop, 1970, 10.1145/362349.362377 (doi).
  ///
  /// \param[in]  center        Center of the sphere
  /// \param[in]  radius        Radius of the sphere
  /// @param[in]  num_agents    The number of agents
  /// @param[in]  agent_builder function containing the logic to instantiate a
  ///                           new agent. Takes `const
  ///                           Double3&` as input parameter
  template <typename Function>
  static void CreateAgentsOnSphereRndm(const Double3& center, double radius,
                                       uint64_t num_agents,
                                       Function agent_builder) {
#pragma omp parallel
    {
      auto* sim = Simulation::GetActive();
      auto* ctxt = sim->GetExecutionContext();
      auto* random = sim->GetRandom();

#pragma omp for
      for (uint64_t i = 0; i < num_agents; i++) {
        auto pos = random->Sphere(radius) + center;
        auto* new_agent = agent_builder(pos);
        ctxt->AddAgent(new_agent);
      }
    }
  }

  /// Creates agents with random positions in a sphere and adds them to the
  /// ExecutionContext. Agents are distributed uniformly inside the sphere.
  /// Agent creation is parallelized. Algorithm: Knop,
  /// 1970, 10.1145/362349.362377 (doi).
  ///
  /// \param[in]  center        Center of the sphere
  /// \param[in]  radius        Radius of the sphere
  /// @param[in]  num_agents    The number of agents
  /// @param[in]  agent_builder function containing the logic to instantiate a
  ///                           new agent. Takes `const
  ///                           Double3&` as input parameter
  template <typename Function>
  static void CreateAgentsInSphereRndm(const Double3& center, double radius,
                                       uint64_t num_agents,
                                       Function agent_builder) {
    // We use a probability density function (PDF) to model the probability of
    // an agent to occur at a distance `r>=0` of the center. As the surface of
    // a sphere scales as `r^2`, the PDF does as well. Thus
    // `p(r)=a*r^2*\Theta(R-r)`, where `\Theta` is a heavyside function and R is
    // largest allowed radius (interpretation: no agents outside the sphere). We
    // can fix `a` by requiring `\int_0^\inf p(r') dr' = 1` and obtain
    // `a=3/R^3`.
    auto radial_pdf_sphere = [](const double* x, const double* params) {
      double R{params[0]};
      double r{x[0]};
      if (r > 0.0 && r <= R) {
        return 3.0 * std::pow(r, 2.0) / std::pow(R, 3.0);
      } else {
        return 0.0;
      }
    };

    // Get a random number generator to sample from our PDF.
    auto* random = Simulation::GetActive()->GetRandom();
    auto rng =
        random->GetUserDefinedDistRng1D(radial_pdf_sphere, {radius}, 0, radius);

    // Create a random radius for each of the agents. Note: this is done
    // serially because we GetUserDefinedDistRng1D does not work in parallel
    // regions at the moment.
    std::vector<double> random_radius;
    random_radius.resize(num_agents);
    for (size_t i = 0; i < num_agents; i++) {
      random_radius[i] = rng.Sample();
    }
#pragma omp parallel shared(random_radius)
    {
      auto* ctxt_tl = Simulation::GetActive()->GetExecutionContext();
#pragma omp for schedule(static)
      for (uint64_t i = 0; i < num_agents; i++) {
        auto pos = random->Sphere(random_radius[i]) + center;
        auto* new_agent = agent_builder(pos);
        ctxt_tl->AddAgent(new_agent);
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
  static void DefineSubstance(size_t substance_id,
                              const std::string& substance_name,
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
