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

#include <array>

#include "core/biology_module/grow_divide.h"
#include "core/container/sim_object_vector.h"
#include "core/environment/environment.h"
#include "core/functor.h"
#include "core/gpu/gpu_helper.h"
#include "core/operation/displacement_op.h"
#include "core/sim_object/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace displacement_op_gpu_test_internal {

// NB: The GPU execution 'context' for the displacement operation differes,
// from the CPU version. Once the CPU version supports the same execution
// context, we can include it for direct comparison of results.

enum ExecutionMode { kCuda, kOpenCl };

static constexpr double kEps = 10 * abs_error<double>::value;

class DisplacementOpCpuVerify {
 public:
  struct CalculateDisplacement;
  struct UpdateCells;

  void operator()() {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    SimObjectVector<Double3> displacements;

    CalculateDisplacement cd(&displacements);
    rm->ApplyOnAllElementsParallelDynamic(1000, cd);
    UpdateCells uc(&displacements);
    rm->ApplyOnAllElementsParallelDynamic(1000, uc);
  }

  struct CalculateDisplacement : public Functor<void, SimObject*, SoHandle> {
    SimObjectVector<Double3>* displacements_;

    CalculateDisplacement(SimObjectVector<Double3>* displacements) {
      displacements_ = displacements;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      auto* sim = Simulation::GetActive();
      auto* env = sim->GetEnvironment();
      auto* param = sim->GetParam();

      auto search_radius = env->GetLargestObjectSize();
      auto squared_radius_ = search_radius * search_radius;
      const auto& displacement = so->CalculateDisplacement(
          squared_radius_, param->simulation_time_step_);
      (*displacements_)[soh] = displacement;
    }
  };

  struct UpdateCells : public Functor<void, SimObject*, SoHandle> {
    SimObjectVector<Double3>* displacements_;

    UpdateCells(SimObjectVector<Double3>* displacements) {
      displacements_ = displacements;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      auto* sim = Simulation::GetActive();
      auto* param = sim->GetParam();

      so->ApplyDisplacement((*displacements_)[soh]);
      if (param->bound_space_) {
        ApplyBoundingBox(so, param->min_bound_, param->max_bound_);
      }
    }
  };
};

void RunTest(ExecutionMode mode) {
  auto set_param = [&](Param* param) {
    switch (mode) {
      case kOpenCl:
        param->use_gpu_ = true;
        param->use_opencl_ = true;
      case kCuda:
        param->use_gpu_ = true;
    }
  };

  enum Case { kCompute, kVerify };

  std::vector<Simulation*> sims;
  sims.push_back(new Simulation("GPU", set_param));
  sims.push_back(new Simulation("CPU_Verify", set_param));
  std::array<SoUid, 2> uid_ref;

  for (size_t i = 0; i < sims.size(); i++) {
    auto& sim = sims[i];
    sim->Activate();
    auto* rm = sim->GetResourceManager();
    auto* env = sim->GetEnvironment();
    uid_ref[i] = SoUid(sim->GetSoUidGenerator()->GetHighestIndex());

    // Cell 0
    Cell* cell = new Cell();
    cell->SetAdherence(0.3);
    cell->SetDiameter(10);
    cell->SetMass(1.4);
    cell->SetPosition({0, 0, 0});
    rm->push_back(cell);

    // Cell 1
    Cell* cell_1 = new Cell();
    cell_1->SetAdherence(0.4);
    cell_1->SetDiameter(10);
    cell_1->SetMass(1.1);
    cell_1->SetPosition({0, 8, 0});
    rm->push_back(cell_1);

    env->Update();

    if (i == Case::kCompute) {
      // Execute operation
      DisplacementOp op;
      op();
    } else {
      // Run verification on CPU
      DisplacementOpCpuVerify cpu_op;
      cpu_op();
    }
  }

  // check results
  auto rm0 = sims[Case::kCompute]->GetResourceManager();
  auto rm1 = sims[Case::kVerify]->GetResourceManager();

  auto cell0 = static_cast<Cell*>(rm0->GetSimObject(uid_ref[0] + 0));
  auto cell1 = static_cast<Cell*>(rm0->GetSimObject(uid_ref[0] + 1));
  auto vcell0 = static_cast<Cell*>(rm1->GetSimObject(uid_ref[1] + 0));
  auto vcell1 = static_cast<Cell*>(rm1->GetSimObject(uid_ref[1] + 1));

  EXPECT_ARR_NEAR_GPU(vcell0->GetPosition(), cell0->GetPosition());
  EXPECT_ARR_NEAR_GPU(vcell1->GetPosition(), cell1->GetPosition());

  // check if tractor_force has been reset to zero
  EXPECT_ARR_NEAR_GPU(cell0->GetTractorForce(), {0, 0, 0});
  EXPECT_ARR_NEAR_GPU(cell1->GetTractorForce(), {0, 0, 0});

  // remaining fields should remain unchanged
  // cell 0
  EXPECT_NEAR(0.3, cell0->GetAdherence(), kEps);
  EXPECT_NEAR(10, cell0->GetDiameter(), kEps);
  EXPECT_NEAR(1.4, cell0->GetMass(), kEps);
  // cell 1
  EXPECT_NEAR(0.4, cell1->GetAdherence(), kEps);
  EXPECT_NEAR(10, cell1->GetDiameter(), kEps);
  EXPECT_NEAR(1.1, cell1->GetMass(), kEps);
}

#ifdef USE_CUDA
TEST(DisplacementOpGpuTest, ComputeSoaCuda) { RunTest(kCuda); }
#endif

#ifdef USE_OPENCL
TEST(DisplacementOpGpuTest, ComputeSoaOpenCL) { RunTest(kOpenCl); }
#endif

void RunTest2(ExecutionMode mode) {
  auto set_param = [&](auto* param) {
    switch (mode) {
      case kOpenCl:
        param->use_gpu_ = true;
        param->use_opencl_ = true;
      case kCuda:
        param->use_gpu_ = true;
    }
  };

  enum Case { kCompute, kVerify };

  std::vector<Simulation*> sims;
  sims.push_back(new Simulation("GPU", set_param));
  sims.push_back(new Simulation("CPU_Verify", set_param));
  std::array<SoUid, 2> uid_ref;

  for (size_t i = 0; i < sims.size(); i++) {
    auto& sim = sims[i];
    sim->Activate();
    auto* rm = sim->GetResourceManager();
    auto* env = sim->GetEnvironment();
    uid_ref[i] = SoUid(sim->GetSoUidGenerator()->GetHighestIndex());

    double space = 20;
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          Cell* cell = new Cell({k * space, j * space, i * space});
          cell->SetDiameter(30);
          cell->SetAdherence(0.4);
          cell->SetMass(1.0);
          rm->push_back(cell);
        }
      }
    }

    env->Update();

    if (i == Case::kCompute) {
      // Execute operation
      DisplacementOp op;
      op();
    } else {
      // Run verification on CPU
      DisplacementOpCpuVerify cpu_op;
      cpu_op();
    }
  }

  // check results
  auto rm0 = sims[Case::kCompute]->GetResourceManager();
  auto rm1 = sims[Case::kVerify]->GetResourceManager();

  // clang-format off
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 0)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 0)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 1)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 1)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 2)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 2)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 3)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 3)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 4)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 4)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 5)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 5)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 6)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 6)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 7)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 7)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 8)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 8)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 9)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 9)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 10)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 10)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 11)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 11)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 12)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 12)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 13)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 13)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 14)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 14)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 15)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 15)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 16)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 16)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 17)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 17)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 18)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 18)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 19)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 19)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 20)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 20)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 21)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 21)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 22)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 22)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 23)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 23)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 24)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 24)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 25)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 25)->GetPosition());
  EXPECT_ARR_NEAR_GPU(rm1->GetSimObject(uid_ref[1] + 26)->GetPosition(), rm0->GetSimObject(uid_ref[0] + 26)->GetPosition());
  // clang-format on
}

#ifdef USE_CUDA
TEST(DisplacementOpGpuTest, ComputeSoaNewCuda) { RunTest2(kCuda); }
#endif

#ifdef USE_OPENCL
TEST(DisplacementOpGpuTest, ComputeSoaNewOpenCL) { RunTest2(kOpenCl); }
#endif

}  // namespace displacement_op_gpu_test_internal
}  // namespace bdm
