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

#include "unit/core/operation/displacement_op_test.h"
#include "gtest/gtest.h"

namespace bdm {
namespace displacement_op_test_internal {

TEST(DisplacementOpTest, ComputeSoa) { RunTest(); }

TEST(DisplacementOpTest, ComputeSoaNew) {
  Simulation simulation(TEST_NAME);
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
  rm->ApplyOnAllElements([&](auto&& sim_object, SoHandle) { op(sim_object); });

  // clang-format off
  EXPECT_ARR_NEAR((*cells)[0].GetPosition(), {-0.20160966809506442, -0.20160966809506442, -0.20160966809506442});
  EXPECT_ARR_NEAR((*cells)[1].GetPosition(), {19.996726567569418, -0.22266672163763598, -0.22266672163763598});
  EXPECT_ARR_NEAR((*cells)[2].GetPosition(), {40.201498173927305, -0.19986951265938641, -0.19986951265938641});
  EXPECT_ARR_NEAR((*cells)[3].GetPosition(), {-0.22107235667371566, 19.99536937981701, -0.22243298066161277});
  EXPECT_ARR_NEAR((*cells)[4].GetPosition(), {19.996445694277536, 19.991084649064845, -0.24302072168423453});
  EXPECT_ARR_NEAR((*cells)[5].GetPosition(), {40.220941141351886, 19.995420431150347, -0.22054684872504365});
  EXPECT_ARR_NEAR((*cells)[6].GetPosition(), {-0.19982869510573351, 40.201437349791846, -0.19959785780667869});
  EXPECT_ARR_NEAR((*cells)[7].GetPosition(), {19.99678197072755, 40.219229813316794, -0.22031963374181529});
  EXPECT_ARR_NEAR((*cells)[8].GetPosition(), {40.199677082903456, 40.19967381378099, -0.19789135788064738});
  EXPECT_ARR_NEAR((*cells)[9].GetPosition(), {-0.2208401632148318, -0.22084860029084805, 19.994023485540392});
  EXPECT_ARR_NEAR((*cells)[10].GetPosition(), {19.996453518945092, -0.2413353410797266, 19.989660774799667});
  EXPECT_ARR_NEAR((*cells)[11].GetPosition(), {40.220709535497228, -0.21898884436167229, 19.994096990730121});
  EXPECT_ARR_NEAR((*cells)[12].GetPosition(), {-0.23959752549368377, 19.995004901057818, 19.989639428390195});
  EXPECT_ARR_NEAR((*cells)[13].GetPosition(), {19.996198530091725, 19.99036483907971, 19.984534769039289});
  EXPECT_ARR_NEAR((*cells)[14].GetPosition(), {40.239451672678122, 19.995058936152933, 19.989784932711256});
  EXPECT_ARR_NEAR((*cells)[15].GetPosition(), {-0.21895170976356038, 40.220651199374309, 19.994102585380165});
  EXPECT_ARR_NEAR((*cells)[16].GetPosition(), {19.996511705095408, 40.237587787915302, 19.989763131796483});
  EXPECT_ARR_NEAR((*cells)[17].GetPosition(), {40.218778612717593, 40.218767244773701, 19.994175099676564});
  EXPECT_ARR_NEAR((*cells)[18].GetPosition(), {-0.19955064743630646, -0.19955761561629054, 40.201369950961137});
  EXPECT_ARR_NEAR((*cells)[19].GetPosition(), {19.99679050342159, -0.22028367546827032, 40.219170471740057});
  EXPECT_ARR_NEAR((*cells)[20].GetPosition(), {40.199399442577274, -0.19785191118676104, 40.199607855810939});
  EXPECT_ARR_NEAR((*cells)[21].GetPosition(), {-0.2186842147430082, 19.995465190535288, 40.218898427254011});
  EXPECT_ARR_NEAR((*cells)[22].GetPosition(), {19.996519321027819, 19.991238784452595, 40.235673745336591});
  EXPECT_ARR_NEAR((*cells)[23].GetPosition(), {40.218511916378013, 19.995514817696375, 40.217044778537549});
  EXPECT_ARR_NEAR((*cells)[24].GetPosition(), {-0.19780590198157336, 40.19933398679354, 40.199323871317965});
  EXPECT_ARR_NEAR((*cells)[25].GetPosition(), {19.996843796541615, 40.216799284843852, 40.216779234925141});
  EXPECT_ARR_NEAR((*cells)[26].GetPosition(), {40.197616806612238, 40.197607143403182, 40.197597121203316});
  // clang-format on
}

}  // namespace displacement_op_test_internal
}  // namespace bdm
