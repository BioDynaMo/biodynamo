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

#include "unit/displacement_op_test.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/default_ctparam.h"

namespace bdm {
namespace displacement_op_test_internal {

// template <typename TBackend = Soa>
// struct SoaCompileTimeParam {
//   template <typename TTBackend>
//   using Self = SoaCompileTimeParam<TTBackend>;
//   using Backend = TBackend;
//   using SimulationBackend = Soa;
//   using BiologyModules = CTList<NullBiologyModule>;
//   using SimObjectTypes = CTList<Cell>;
// };

TEST(DisplacementOpTest, ComputeSoa) { RunTest(); }

TEST(DisplacementOpTest, ComputeSoaNew) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  const auto* cells = rm->template Get<Cell>();

  double space = 20;
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      for (size_t k = 0; k < 3; k++) {
        Cell cell({k * space, j * space, i * space});
        cell.SetDiameter(30);
        cell.SetAdherence(0.4);
        cell.SetMass(1.0);
        rm->push_back(cell);
      }
    }
  }

  grid->ClearGrid();
  grid->Initialize();

  // execute operation
  DisplacementOp<> op;
  op();

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

}  // namespace displacement_op_test_internal
}  // namespace bdm
