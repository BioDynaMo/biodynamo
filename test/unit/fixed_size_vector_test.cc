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

#include "fixed_size_vector.h"
#include "gtest/gtest.h"
#include "bdm_imp.h"

namespace bdm {

TEST(FixedSizeVector, All) {
  FixedSizeVector<int, 4> vector;

  ASSERT_EQ(0u, vector.size());

  vector.push_back(0);
  ASSERT_EQ(1u, vector.size());

  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  ASSERT_EQ(4u, vector.size());

  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(i, vector[i]);
  }

  for (int i = 0; i < 4; i++) {
    vector[i]++;
  }

  int counter = 0;
  for (auto& element : vector) {
    ASSERT_EQ(counter + 1, element);
    counter++;
  }

  ++vector;
  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(i + 2, vector[i]);
  }

  vector.clear();
  ASSERT_EQ(0u, vector.size());
}

}  // namespace bdm
