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
#ifndef BDM_EX1_H_
#define BDM_EX1_H_

#include "biodynamo.h"

#include "core/behavior/regulatory_network.h"

namespace bdm {

enum Substances { kProtein };

#ifndef __ROOTCLING__
struct ODE_system {
  const std::map<std::string, DiffusionGrid*>& mdg;
  const std::vector<real_t> param;

  ODE_system(std::map<std::string, DiffusionGrid*>& m,
             const std::vector<real_t>& p)
      : mdg(m), param(p) {}

  void operator()(const b_vector_t& x, b_vector_t& dxdt, real_t t,
                  Agent* agent) const {
    auto& xyz = agent->GetPosition();
    auto dg = mdg.find("protein")->second;
    const real_t protein = dg->GetValue(xyz);

    dxdt[0] = t * x[0] + param[0];
    dxdt[1] = t * x[1] * x[1] + param[1] * protein;
    dxdt[2] = t + x[2] + param[2];
  }
};

struct ODE_jacobian {
  const std::map<std::string, DiffusionGrid*>& mdg;
  const std::vector<real_t> param;

  ODE_jacobian(std::map<std::string, DiffusionGrid*>& m,
               const std::vector<real_t>& p)
      : mdg(m), param(p) {}

  void operator()(const b_vector_t& x, b_matrix_t& jac, real_t t,
                  b_vector_t& dfdt, Agent* agent) const {
    // auto& xyz = agent->GetPosition();
    // auto dg = mdg.find("protein")->second;
    // const real_t protein = dg->GetValue(xyz);

    jac(0, 0) = t;
    jac(0, 1) = 0.0;
    jac(0, 2) = 0.0;
    jac(1, 0) = 0.0;
    jac(1, 1) = t * 2.0 * x[1];
    jac(1, 2) = 0.0;
    jac(2, 0) = 0.0;
    jac(2, 1) = 0.0;
    jac(2, 2) = 1.0;
    //
    dfdt[0] = dfdt[1] = dfdt[2] = 0.0;
  }
};

struct ODE_output {
  void operator()(const b_vector_t& x, real_t t, const Agent* agent) {
    auto& xyz = agent->GetPosition();
    std::clog << agent->GetUid() << ',' << xyz[0] << ',' << xyz[1] << ','
              << xyz[2] << ',' << t << ',' << x[0] << ',' << x[1] << ','
              << x[2];
    std::clog << std::endl;
  }
};
#endif

namespace ex1 {

inline int Simulate(int argc, const char** argv) {
  std::ofstream fout("ex1.csv");
  // save the original buffer of std::clog
  std::streambuf* orig_clog_buff = std::clog.rdbuf();
  // redirect std::clog to point to the above file
  std::clog.rdbuf(fout.rdbuf());

  // set-up the BioDynaMo simulation parameters
  auto set_parameters = [](Param* param) {
    param->output_dir = "ex1";
    param->use_progress_bar = false;
    param->bound_space = Param::BoundSpaceMode::kOpen;
    param->min_bound = 0.0;
    param->max_bound = 100.0;
    param->export_visualization = true;
    param->visualization_interval = 1;
    param->visualize_agents["Cell"] = {"diameter_", "volume_"};
    param->statistics = false;
    param->simulation_time_step = 1.0;
    param->visualize_diffusion = {
        Param::VisualizeDiffusion{"protein", true, true}};
    param->calculate_gradients = false;
    param->diffusion_method = "euler";
  };

  Simulation sim(argc, argv, set_parameters);

  auto* rm = sim.GetResourceManager();

  // time-step of the BioDynaMo simulator
  const real_t dt_BDM = sim.GetParam()->simulation_time_step;
  // time-step of the regulatory network solver
  const real_t dt_RN = 0.01;
  // BioDynaMo's diffusion grid sample points in each dimension
  int n_DG = 51;

  ModelInitializer::DefineSubstance(kProtein, "protein", 0.0, 0.0, n_DG);
  ModelInitializer::AddBoundaryConditions(
      kProtein, BoundaryConditionType::kNeumann,
      std::make_unique<ConstantBoundaryCondition>(0));

  std::map<std::string, DiffusionGrid*> dg_map;
  dg_map.insert(std::make_pair("protein", rm->GetDiffusionGrid("protein")));

  {
    Real3 xyz{0.0, 0.0, 0.0};

    Cell* c = new Cell();
    c->SetDiameter(1.0);
    c->SetAdherence(0.4);
    c->SetMass(1.0);
    c->SetPosition(xyz);
#ifndef __ROOTCLING__
    c->AddBehavior(new RegulatoryNetwork(
        dt_RN, 1000, {1., 5., 7.},
        // ODE_solver::Euler,
        // ODE_solver::Rosenbrock,
        ODE_solver::RungeKutta, ODE_system(dg_map, {0.2, 0.1, 3.0}),
        ODE_jacobian(dg_map, {0.2, 0.1, 3.0}), ODE_output()));
#endif

    sim.GetExecutionContext()->AddAgent(c);
  }

  for (int s = 0; s < 10; s++)
    sim.GetScheduler()->Simulate(1);

  // restore the original buffer of std::clog
  std::clog.rdbuf(orig_clog_buff);

  return 0;
}

}  // namespace ex1

}  // namespace bdm

#endif  // BDM_EX1_H_
