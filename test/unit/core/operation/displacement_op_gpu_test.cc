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

#include "core/biology_module/grow_divide.h"
#include "core/gpu/gpu_helper.h"
#include "core/grid.h"
#include "core/operation/displacement_op.h"
#include "core/sim_object/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace displacement_op_gpu_test_internal {

enum ExecutionMode { kCuda, kOpenCl };

void RunTest(ExecutionMode mode) {
  auto set_param = [&](Param* param) {
    switch (mode) {
      case kOpenCl:
        param->use_opencl_ = true;
      case kCuda:
        param->use_gpu_ = true;
    }
  };
  Simulation simulation("displacement_op_gpu_test_RunTest", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  // Do this explicitly because this normally is only called in
  // Scheduler::Initialize(), but in this test we call DisplacementOp directly.
  InitializeGPUEnvironment<>();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  // Cell 0
  Cell* cell = new Cell();
  cell->SetAdherence(0.3);
  cell->SetDiameter(9);
  cell->SetMass(1.4);
  cell->SetPosition({0, 0, 0});
  cell->SetPosition({0, 0, 0});
  rm->push_back(cell);

  // Cell 1
  Cell* cell_1 = new Cell();
  cell_1->SetAdherence(0.4);
  cell_1->SetDiameter(11);
  cell_1->SetMass(1.1);
  cell_1->SetPosition({0, 5, 0});
  cell_1->SetPosition({0, 5, 0});
  rm->push_back(cell_1);

  grid->Initialize();

  // execute operation
  DisplacementOp op;
  op();

  // check results
  // cell 0
  Cell* final_cell0 = dynamic_cast<Cell*>(rm->GetSimObject(ref_uid + 0));
  Cell* final_cell1 = dynamic_cast<Cell*>(rm->GetSimObject(ref_uid + 1));
  auto final_position = final_cell0->GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 1
  final_position = final_cell1->GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0992371702325645, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 0
  auto final_tf = final_cell0->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 1
  final_tf = final_cell1->GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 0
  EXPECT_NEAR(0.3, final_cell0->GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, final_cell0->GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, final_cell0->GetMass(), abs_error<double>::value);
  // cell 1
  EXPECT_NEAR(0.4, final_cell1->GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, final_cell1->GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, final_cell1->GetMass(), abs_error<double>::value);
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
        param->use_opencl_ = true;
      case kCuda:
        param->use_gpu_ = true;
    }
  };
  Simulation simulation("DisplacementOpGpuTest_RunTest2", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  // Do this explicitly because this normally is only called in
  // Scheduler::Initialize(), but in this test we call DisplacementOp directly.
  InitializeGPUEnvironment<>();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

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

  grid->ClearGrid();
  grid->Initialize();

  // execute operation
  DisplacementOp op;
  op();

  // clang-format off
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 0)->GetPosition(), {-0.20160966809506442, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 1)->GetPosition(), {20, -0.22419529008561653, -0.22419529008561653});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 2)->GetPosition(), {40.201609668095067, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 3)->GetPosition(), {-0.22419529008561653, 20, -0.22419529008561653});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 4)->GetPosition(), {20, 20, -0.24678091207616867});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 5)->GetPosition(), {40.224195290085618, 20, -0.22419529008561653});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 6)->GetPosition(), {-0.20160966809506442, 40.201609668095067, -0.20160966809506442});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 7)->GetPosition(), {20, 40.224195290085618, -0.22419529008561653});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 8)->GetPosition(), {40.201609668095067, 40.201609668095067, -0.20160966809506442});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 9)->GetPosition(), {-0.22419529008561653, -0.22419529008561653, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 10)->GetPosition(), {20, -0.24678091207616867, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 11)->GetPosition(), {40.224195290085618, -0.22419529008561653, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 12)->GetPosition(), {-0.24678091207616867, 20, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 13)->GetPosition(), {20, 20, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 14)->GetPosition(), {40.246780912076169, 20, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 15)->GetPosition(), {-0.22419529008561653, 40.224195290085618, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 16)->GetPosition(), {20, 40.246780912076169, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 17)->GetPosition(), {40.224195290085618, 40.224195290085618, 20});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 18)->GetPosition(), {-0.20160966809506442, -0.20160966809506442, 40.201609668095067});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 19)->GetPosition(), {20, -0.22419529008561653, 40.224195290085618});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 20)->GetPosition(), {40.201609668095067, -0.20160966809506442, 40.201609668095067});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 21)->GetPosition(), {-0.22419529008561653, 20, 40.224195290085618});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 22)->GetPosition(), {20, 20, 40.246780912076169});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 23)->GetPosition(), {40.224195290085618, 20, 40.224195290085618});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 24)->GetPosition(), {-0.20160966809506442, 40.201609668095067, 40.201609668095067});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 25)->GetPosition(), {20, 40.224195290085618, 40.224195290085618});
  EXPECT_ARR_NEAR(rm->GetSimObject(ref_uid + 26)->GetPosition(), {40.201609668095067, 40.201609668095067, 40.201609668095067});
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
