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
#ifndef BDM_EX2_H_
#define BDM_EX2_H_

#include "biodynamo.h"

#include "core/agent/agent.h"
#include "core/behavior/regulatory_network.h"

namespace bdm {

class MyCell : public Cell {
  BDM_AGENT_HEADER(MyCell, Cell, 1);

 public:
  MyCell() {}
  explicit MyCell(const Real3& position) : Base(position), trail_(0.0) {}
  virtual ~MyCell() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (auto* mother = dynamic_cast<MyCell*>(event.existing_agent)) {
      if (event.GetUid() == CellDivisionEvent::kUid) {
        // copy properties from mother to daughter
      }
    }
  }

  real_t GetTrail() const { return trail_; }
  void SetTrail(real_t t) { trail_ += t; }

 private:
  /// keep track of the trail of the agent
  real_t trail_;
};

#ifndef __ROOTCLING__
struct Lorenz_rhs_ {
  void operator()(const b_vector_t& x, b_vector_t& dxdt, double t,
                  Agent* agent) const {
    dxdt[0] = sigma * x[1] - sigma * x[0];
    dxdt[1] = rho * x[0] - x[1] - x[0] * x[2];
    dxdt[2] = -beta * x[2] + x[0] * x[1];
  }
  // https://en.wikipedia.org/wiki/Lorenz_system
  double sigma = 10.0;
  double rho = 28.0;
  double beta = 8.0 / 3.0;
};

struct Lorenz_jac_ {
  void operator()(const b_vector_t& x, b_matrix_t& jac, double t,
                  b_vector_t& dfdt, Agent* agent) const {
    jac(0, 0) = -sigma;
    jac(0, 1) = sigma;
    jac(0, 2) = 0.0;
    jac(1, 0) = rho - x[2];
    jac(1, 1) = -1.0;
    jac(1, 2) = -x[0];
    jac(2, 0) = x[1];
    jac(2, 1) = x[0];
    jac(2, 2) = -beta;
    //
    dfdt[0] = dfdt[1] = dfdt[2] = 0.0;
  }
  // https://en.wikipedia.org/wiki/Lorenz_system
  double sigma = 10.0;
  double rho = 28.0;
  double beta = 8.0 / 3.0;
};

struct Lorenz_out_ {
  void operator()(const b_vector_t& x, real_t t, const Agent* agent) {
    auto& xyz = agent->GetPosition();
    std::clog << agent->GetUid() << ',' << xyz[0] << ',' << xyz[1] << ','
              << xyz[2] << ',' << t << ',' << x[0] << ',' << x[1] << ','
              << x[2];
    std::clog << std::endl;
  }
};
#endif

class Trajectory : public RegulatoryNetwork {
  BDM_BEHAVIOR_HEADER(Trajectory, RegulatoryNetwork, 1);

 public:
  Trajectory() { AlwaysCopyToNew(); }
#ifndef __ROOTCLING__
  Trajectory(real_t dt, int n_dt, const std::vector<real_t>& x)
      : RegulatoryNetwork(dt, n_dt, x, ODE_solver::Rosenbrock, Lorenz_rhs_(),
                          Lorenz_jac_(), Lorenz_out_()) {}
#endif
  virtual ~Trajectory() = default;

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
  }

  void Run(Agent* agent) override {
    Base::Run(agent);

#ifndef __ROOTCLING__
    Real3 xyz;
    for (int i = 0; i < 3; i++)
      xyz[i] = this->GetSpecie(i);

    if (auto* cell = dynamic_cast<MyCell*>(agent)) {
      Real3 diff = xyz - cell->GetPosition();
      // calculate the cell trail
      cell->SetTrail(diff.Norm());
      // now set its new position
      cell->SetPosition(xyz);
    } else {
      Log::Fatal("Trajectory::Run", "agent is not of 'MyCell' type");
    }
#endif
  }
};

namespace ex2 {

inline int Simulate(int argc, const char** argv) {
  std::ofstream fout("ex2.csv");
  // save the original buffer of std::clog
  std::streambuf* orig_clog_buff = std::clog.rdbuf();
  // redirect std::clog to point to the above file
  std::clog.rdbuf(fout.rdbuf());

  // set-up the BioDynaMo simulation parameters
  auto set_parameters = [](Param* param) {
    param->output_dir = "ex2";
    param->use_progress_bar = false;
    param->bound_space = Param::BoundSpaceMode::kOpen;
    param->min_bound = -1000.0;
    param->max_bound = +1000.0;
    param->export_visualization = true;
    param->visualization_interval = 1;
    param->visualize_agents["MyCell"] = {"diameter_", "volume_", "trail_"};
    param->statistics = false;
    param->simulation_time_step = 1.0;
  };

  Simulation sim(argc, argv, set_parameters);

  // time-step of the regulatory network solver
  const real_t dt_RN = 0.01;

  {
    Real3 xyz{1.0, 1.0, 1.0};

    MyCell* c = new MyCell();
    c->SetDiameter(1.0);
    c->SetPosition(xyz);
#ifndef __ROOTCLING__
    c->AddBehavior(new Trajectory(dt_RN, 222, {xyz[0], xyz[1], xyz[2]}));
#else
    c->AddBehavior(new Trajectory());
#endif

    sim.GetExecutionContext()->AddAgent(c);
  }

  for (int s = 0; s < 3000; s++)
    sim.GetScheduler()->Simulate(1);

  // restore the original buffer of std::clog
  std::clog.rdbuf(orig_clog_buff);

  return 0;
}

}  // namespace ex2

}  // namespace bdm

#endif  // BDM_EX2_H_
