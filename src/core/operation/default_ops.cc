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

#include <iostream>
#include "core/analysis/time_series.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/continuum_op.h"
#include "core/operation/dividing_cell_op.h"
#include "core/operation/load_balancing_op.h"
#include "core/operation/mechanical_forces_op.h"
#include "core/operation/mechanical_forces_op_cuda.h"
#include "core/operation/mechanical_forces_op_opencl.h"
#include "core/operation/operation.h"
#include "core/operation/visualization_op.h"

namespace bdm {

BDM_REGISTER_OP(BoundSpace, "bound space", kCpu);

BDM_REGISTER_OP(ContinuumOp, "continuum", kCpu);

// By default run load balancing only in the first iteration.
BDM_REGISTER_OP_WITH_FREQ(LoadBalancingOp, "load balancing", kCpu,
                          std::numeric_limits<uint32_t>::max());

BDM_REGISTER_OP(MechanicalForcesOp, "mechanical forces", kCpu);

#ifdef USE_CUDA
BDM_REGISTER_OP(MechanicalForcesOpCuda, "mechanical forces", kCuda);
#endif

BDM_REGISTER_OP(DividingCellOp, "DividingCellOp", kCpu);

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
BDM_REGISTER_OP(MechanicalForcesOpOpenCL, "mechanical forces", kOpenCl);
#endif

struct UpdateStaticnessOp : public AgentOperationImpl {
  BDM_OP_HEADER(UpdateStaticnessOp);

  void operator()(Agent* agent) override { agent->UpdateStaticness(); }
};

BDM_REGISTER_OP(UpdateStaticnessOp, "update staticness", kCpu);

struct PropagateStaticnessAgentOp : public AgentOperationImpl {
  BDM_OP_HEADER(PropagateStaticnessAgentOp);

  void operator()(Agent* agent) override { agent->PropagateStaticness(); }
};

BDM_REGISTER_OP(PropagateStaticnessAgentOp, "propagate staticness agentop",
                kCpu);

struct BehaviorOp : public AgentOperationImpl {
  BDM_OP_HEADER(BehaviorOp);

  void operator()(Agent* agent) override { agent->RunBehaviors(); }
};

BDM_REGISTER_OP(BehaviorOp, "behavior", kCpu);

struct DiscretizationOp : public AgentOperationImpl {
  BDM_OP_HEADER(DiscretizationOp);

  void operator()(Agent* agent) override { agent->RunDiscretization(); }
};

BDM_REGISTER_OP(DiscretizationOp, "discretization", kCpu);

struct SetUpIterationOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(SetUpIterationOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
    sim->GetEnvironment()->Update();
  }
};

BDM_REGISTER_OP(SetUpIterationOp, "set up iteration", kCpu);

struct TearDownIterationOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(TearDownIterationOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->TearDownIterationAll(all_exec_ctxts);
  }
};

BDM_REGISTER_OP(TearDownIterationOp, "tear down iteration", kCpu);

struct UpdateTimeSeriesOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(UpdateTimeSeriesOp);

  void operator()() override {
    Simulation::GetActive()->GetTimeSeries()->Update();
  }
};

BDM_REGISTER_OP(UpdateTimeSeriesOp, "update time series", kCpu);

struct UpdateEnvironmentOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(UpdateEnvironmentOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    sim->GetEnvironment()->ForcedUpdate();
  }
};

BDM_REGISTER_OP(UpdateEnvironmentOp, "update environment", kCpu);

BDM_REGISTER_OP(VisualizationOp, "visualize", kCpu);

struct PropagateStaticnessOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(PropagateStaticnessOp);

  void operator()() override {
    if (!Simulation::GetActive()->GetParam()->detect_static_agents) {
      return;
    }
    auto function = L2F(
        [](Agent* agent, AgentHandle) { agent->PropagateStaticness(true); });
    auto* rm = Simulation::GetActive()->GetResourceManager();
    auto* param = Simulation::GetActive()->GetParam();
    rm->ForEachAgentParallel(param->scheduling_batch_size, function);
  }
};

BDM_REGISTER_OP(PropagateStaticnessOp, "propagate staticness", kCpu);


//struct export_mass : public StandaloneOperationImpl {
//  BDM_OP_HEADER(export_mass);
//
//  void operator()() override {
//    auto* sim = Simulation::GetActive();
//    auto* rm = sim->GetResourceManager();
//
//    real_t total_HET = 0.0;
//    real_t total_EPS = 0.0;
//
//    rm->ForEachAgent([&](Agent* agent) {
//        auto* cell = dynamic_cast<Cell*>(agent);
//        real_t mass = cell->GetMass();
//	if (cell->GetCellType() == "HET") {
//	    auto* HET = dynamic_cast<HET*>(agent);
//	    total_HET += mass;
//	}
//	if (cell->GetCellType() == "EPS") {
//	    auto* EPS = dynamic_cast<EPS*>(agent);
//            total_EPS += mass;
//        }
//    });
//
//    real_t total = total_HET + total_EPS;
//
//    int t = sim->GetScheduler()->GetSimulatedSteps();
//    std::ofstream outdata;
//
//    outdata.open("export_measurements/mass/mass_" + std::to_string(t) + ".txt");
//    outdata << total << std::endl;
//    outdata << total_HET << std::endl;
//    outdata << total_EPS << std::endl;
//    outdata.close();
//  }
//};
//
//BDM_REGISTER_OP(export_mass, "export mass", kCpu);
//
//
//
//struct export_conc : public StandaloneOperationImpl {
//  BDM_OP_HEADER(export_conc);
//
//  void operator()() override {
//    auto* sim = Simulation::GetActive();
//    
//    auto* dgrid_o2 = sim->GetResourceManager()->GetDiffusionGrid( "Oxygen" );
//    real_t conc_o2 = 0.0;
//    int box_num_o2 = dgrid_o2->GetNumBoxes();
//    for (int i=0; i<box_num_o2; i++) {
//	    conc_o2 += dgrid_o2->GetValue(i);
//    }
//
//    auto* dgrid_s = sim->GetResourceManager()->GetDiffusionGrid( "Substrate" );
//    real_t conc_s = 0.0;
//    int box_num_s = dgrid_s->GetNumBoxes();
//    for (int i=0; i<box_num_s; i++) {
//            conc_s += dgrid_s->GetValue(i);
//    }
//
//    auto* dgrid_co2 = sim->GetResourceManager()->GetDiffusionGrid( "CO2" );
//    real_t conc_co2 = 0.0;
//    int box_num_co2 = dgrid_co2->GetNumBoxes();
//    for (int i=0; i<box_num_co2; i++) {
//            conc_co2 += dgrid_co2->GetValue(i);
//    }
//
//    int t = sim->GetScheduler()->GetSimulatedSteps();
//    std::ofstream outdata;
//
//    outdata.open("export_measurements/conc/conc_" + std::to_string(t) + ".txt");
//    outdata << conc_o2 << std::endl;
//    outdata << conc_s << std::endl;
//    outdata << conc_co2 << std::endl;
//    outdata.close();
//
//  }
//};
//
//BDM_REGISTER_OP(export_conc, "export conc", kCpu);



}  // namespace bdm
