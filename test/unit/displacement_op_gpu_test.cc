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

#include "biology_module/grow_divide.h"
#include "cell.h"
#include "displacement_op.h"
#include "gpu/gpu_helper.h"
#include "grid.h"
#include "gtest/gtest.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"
#include "simulation_implementation.h"

namespace bdm {
namespace displacement_op_gpu_test_internal {

enum ExecutionMode { kCpu, kCuda, kOpenCl };

void RunTest(ExecutionMode mode) {
  Simulation<> simulation("displacement_op_gpu_test_RunTest");
  auto* rm = simulation.GetRm();
  auto* grid = simulation.GetGrid();
  auto* param = simulation.GetParam();

  switch(mode) {
    case kOpenCl: param->use_opencl_ = true;
    case kCuda: param->use_gpu_ = true;
  }

  auto cells = rm->template Get<Cell>();

  // Cell 1
  Cell cell;
  cell.SetAdherence(0.3);
  cell.SetDiameter(9);
  cell.SetMass(1.4);
  cell.SetPosition({0, 0, 0});
  cell.SetPosition({0, 0, 0});
  cells->push_back(cell);

  // Cell 2
  cell.SetAdherence(0.4);
  cell.SetDiameter(11);
  cell.SetMass(1.1);
  cell.SetPosition({0, 5, 0});
  cell.SetPosition({0, 5, 0});
  cells->push_back(cell);

  grid->Initialize();

  // execute operation
  DisplacementOp<> op;
  op(cells, 0);

  // check results
  // cell 1
  auto final_position = (*cells)[0].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 2
  final_position = (*cells)[1].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0992371702325645, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 1
  auto final_tf = (*cells)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 2
  final_tf = (*cells)[1].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 1
  EXPECT_NEAR(0.3, (*cells)[0].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, (*cells)[0].GetMass(), abs_error<double>::value);
  // cell 2
  EXPECT_NEAR(0.4, (*cells)[1].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, (*cells)[1].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, (*cells)[1].GetMass(), abs_error<double>::value);
}

#ifdef USE_CUDA
TEST(DisplacementOpGpuTest, ComputeSoaCuda) {
  RunTest(kCuda);
}
#endif

#ifdef USE_OPENCL
TEST(DisplacementOpGpuTest, ComputeSoaOpenCL) {
  RunTest(kOpenCl);
}
#endif

void RunTest2(ExecutionMode mode) {
  Simulation<> simulation("DisplacementOpGpuTest_RunTest2");
  auto* rm = simulation.GetRm();
  auto* grid = simulation.GetGrid();
  auto* param = simulation.GetParam();

  switch(mode) {
    case kOpenCl: param->use_opencl_ = true;
    case kCuda: param->use_gpu_ = true;
  }

  auto cells = rm->template Get<Cell>();

  double space = 20;
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      for (size_t k = 0; k < 3; k++) {
        Cell cell({k * space, j * space, i * space});
        cell.SetDiameter(30);
        cell.SetAdherence(0.4);
        cell.SetMass(1.0);
        cells->push_back(cell);
      }
    }
  }

  grid->ClearGrid();
  grid->Initialize();

  // execute operation
  DisplacementOp<> op;
  op(cells, 0);

  // clang-format off
  EXPECT_ARR_NEAR((*cells)[0].GetPosition(), {-0.20160966809506442, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR((*cells)[1].GetPosition(), {20, -0.22419529008561653, -0.22419529008561653});
  EXPECT_ARR_NEAR((*cells)[2].GetPosition(), {40.201609668095067, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR((*cells)[3].GetPosition(), {-0.22419529008561653, 20, -0.22419529008561653});
  EXPECT_ARR_NEAR((*cells)[4].GetPosition(), {20, 20, -0.24678091207616867});
  EXPECT_ARR_NEAR((*cells)[5].GetPosition(), {40.224195290085618, 20, -0.22419529008561653});
  EXPECT_ARR_NEAR((*cells)[6].GetPosition(), {-0.20160966809506442, 40.201609668095067, -0.20160966809506442});
  EXPECT_ARR_NEAR((*cells)[7].GetPosition(), {20, 40.224195290085618, -0.22419529008561653});
  EXPECT_ARR_NEAR((*cells)[8].GetPosition(), {40.201609668095067, 40.201609668095067, -0.20160966809506442});
  EXPECT_ARR_NEAR((*cells)[9].GetPosition(), {-0.22419529008561653, -0.22419529008561653, 20});
  EXPECT_ARR_NEAR((*cells)[10].GetPosition(), {20, -0.24678091207616867, 20});
  EXPECT_ARR_NEAR((*cells)[11].GetPosition(), {40.224195290085618, -0.22419529008561653, 20});
  EXPECT_ARR_NEAR((*cells)[12].GetPosition(), {-0.24678091207616867, 20, 20});
  EXPECT_ARR_NEAR((*cells)[13].GetPosition(), {20, 20, 20});
  EXPECT_ARR_NEAR((*cells)[14].GetPosition(), {40.246780912076169, 20, 20});
  EXPECT_ARR_NEAR((*cells)[15].GetPosition(), {-0.22419529008561653, 40.224195290085618, 20});
  EXPECT_ARR_NEAR((*cells)[16].GetPosition(), {20, 40.246780912076169, 20});
  EXPECT_ARR_NEAR((*cells)[17].GetPosition(), {40.224195290085618, 40.224195290085618, 20});
  EXPECT_ARR_NEAR((*cells)[18].GetPosition(), {-0.20160966809506442, -0.20160966809506442, 40.201609668095067});
  EXPECT_ARR_NEAR((*cells)[19].GetPosition(), {20, -0.22419529008561653, 40.224195290085618});
  EXPECT_ARR_NEAR((*cells)[20].GetPosition(), {40.201609668095067, -0.20160966809506442, 40.201609668095067});
  EXPECT_ARR_NEAR((*cells)[21].GetPosition(), {-0.22419529008561653, 20, 40.224195290085618});
  EXPECT_ARR_NEAR((*cells)[22].GetPosition(), {20, 20, 40.246780912076169});
  EXPECT_ARR_NEAR((*cells)[23].GetPosition(), {40.224195290085618, 20, 40.224195290085618});
  EXPECT_ARR_NEAR((*cells)[24].GetPosition(), {-0.20160966809506442, 40.201609668095067, 40.201609668095067});
  EXPECT_ARR_NEAR((*cells)[25].GetPosition(), {20, 40.224195290085618, 40.224195290085618});
  EXPECT_ARR_NEAR((*cells)[26].GetPosition(), {40.201609668095067, 40.201609668095067, 40.201609668095067});
  // clang-format on
}

#ifdef USE_CUDA
TEST(DisplacementOpGpuTest, ComputeSoaNewCuda) {
  RunTest2(kCuda);
}
#endif

#ifdef USE_OPENCL
TEST(DisplacementOpGpuTest, ComputeSoaNewOpenCL) {
  RunTest2(kOpenCl);
}
#endif

}  // namespace displacement_op_gpu_test_internal
}  // namespace bdm
