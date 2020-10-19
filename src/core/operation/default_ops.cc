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

#include "core/operation/bound_space_op.h"
#include "core/operation/diffusion_op.h"
#include "core/operation/displacement_op.h"
#include "core/operation/displacement_op_cuda.h"
#include "core/operation/displacement_op_opencl.h"
#include "core/operation/dividing_cell_op.h"
#include "core/operation/load_balancing_op.h"
#include "core/operation/operation.h"
#include "core/operation/visualization_op.h"

namespace bdm {

BDM_REGISTER_OP(BoundSpace, "bound space", kCpu);

BDM_REGISTER_OP(DiffusionOp, "diffusion", kCpu);

// By default run load balancing only in the first iteration.
BDM_REGISTER_OP_WITH_FREQ(LoadBalancingOp, "load balancing", kCpu,
                          std::numeric_limits<std::size_t>::max());

BDM_REGISTER_OP(DisplacementOp, "displacement", kCpu);

#ifdef USE_CUDA
BDM_REGISTER_OP(DisplacementOpCuda, "displacement", kCuda);
#endif

BDM_REGISTER_OP(DividingCellOp, "DividingCellOp", kCpu);

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
BDM_REGISTER_OP(DisplacementOpOpenCL, "displacement", kOpenCl);
#endif

struct UpdateRunDisplacementOp : public SimObjectOperationImpl {
  BDM_OP_HEADER(UpdateRunDisplacementOp);

  void operator()(SimObject* so) override { so->UpdateRunDisplacement(); }
};

BDM_REGISTER_OP(UpdateRunDisplacementOp, "update run displacement", kCpu);

struct DistributeRunDisplacementInfoOp : public SimObjectOperationImpl {
  BDM_OP_HEADER(DistributeRunDisplacementInfoOp);
  
  bool detect_static_sim_objects_ = false;
  DistributeRunDisplacementInfoOp() {}
  DistributeRunDisplacementInfoOp(const DistributeRunDisplacementInfoOp&) {
    detect_static_sim_objects_ = Simulation::GetActive()->GetParam()->detect_static_sim_objects_;
  }

  void operator()(SimObject* so) override {
    so->DistributeRunDisplacementInfo(detect_static_sim_objects_);
  }
};

BDM_REGISTER_OP(DistributeRunDisplacementInfoOp,
                "distribute run displacement info", kCpu);

struct BiologyModuleOp : public SimObjectOperationImpl {
  BDM_OP_HEADER(BiologyModuleOp);

  void operator()(SimObject* so) override { so->RunBiologyModules(); }
};

BDM_REGISTER_OP(BiologyModuleOp, "biology module", kCpu);

struct DiscretizationOp : public SimObjectOperationImpl {
  BDM_OP_HEADER(DiscretizationOp);

  void operator()(SimObject* so) override { so->RunDiscretization(); }
};

BDM_REGISTER_OP(DiscretizationOp, "discretization", kCpu);

struct SetUpIterationOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(SetUpIterationOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    const auto& all_exec_ctxts = sim->GetAllExecCtxts();
    all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
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

struct UpdateEnvironmentOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(UpdateEnvironmentOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    auto* env = sim->GetEnvironment();
    env->Update();
  }
};

BDM_REGISTER_OP(UpdateEnvironmentOp, "update environment", kCpu);

BDM_REGISTER_OP(VisualizationOp, "visualize", kCpu);

}  // namespace bdm
