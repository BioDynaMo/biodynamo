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

#include "core/biology_module/chemotaxis.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace chemotaxis_test_ns {

struct TestDiffusionGrid : public DiffusionGrid {
  TestDiffusionGrid(const Double3& normalized_gradient)
      : DiffusionGrid(0, "TestSubstance", 1, 1),
        normalized_gradient_(normalized_gradient) {}

  void GetGradient(const Double3& position, Double3* gradient) const override {
    (*gradient) = normalized_gradient_;
  }

  Double3 normalized_gradient_;
};

TEST(ChemotaxisTest, Run) {
  Simulation simulation(TEST_NAME);

  Cell cell;
  Double3 pos = {10, 10, 10};
  cell.SetPosition(pos);
  cell.SetDiameter(40);

  Double3 normalized_gradient = {1, 2, 3};
  normalized_gradient.Normalize();
  TestDiffusionGrid dgrid(normalized_gradient);

  Chemotaxis ct(&dgrid, 3.14);
  ct.Run(&cell);

  EXPECT_ARR_NEAR(pos + normalized_gradient * 3.14, cell.GetPosition());
}

TEST(ChemotaxisTest, EventCopy) {
  auto event_id1 = UniqueEventIdFactory::Get()->NewUniqueEventId();
  auto event_id2 = UniqueEventIdFactory::Get()->NewUniqueEventId();
  auto event_id3 = UniqueEventIdFactory::Get()->NewUniqueEventId();
  auto event_id4 = UniqueEventIdFactory::Get()->NewUniqueEventId();

  Chemotaxis ct(nullptr, 3.14, {event_id1}, {event_id4});
  EXPECT_TRUE(ct.Copy(event_id1));
  EXPECT_FALSE(ct.Copy(event_id2));
  EXPECT_FALSE(ct.Remove(event_id3));
  EXPECT_TRUE(ct.Remove(event_id4));
}

}  // namespace chemotaxis_test_ns
}  // namespace bdm
