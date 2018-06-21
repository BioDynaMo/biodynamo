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

#include "random.h"
#include <gtest/gtest.h>
#include "unit/io_test.h"

namespace bdm {

TEST(RandomTest, Uniform) {
  Random random;
  TRandom3 reference;

  random.SetSeed(42);
  reference.SetSeed(42);

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(), random.Uniform());
  }

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i), random.Uniform(i));
  }

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i, i+2), random.Uniform(i, i + 2));
  }
}

TEST(RandomTest, UniformArray) {
  Random random;
  TRandom3 reference;

  random.SetSeed(42);
  reference.SetSeed(42);

  auto result = random.UniformArray<5>();
  for(uint64_t i = 0; i < 5; i++) {
    EXPECT_EQ(reference.Uniform(), result[i]);
  }

  auto result1 = random.UniformArray<2>(8.3);
  for(uint64_t i = 0; i < 2; i++) {
    EXPECT_EQ(reference.Uniform(8.3), result1[i]);
  }

  auto result2 = random.UniformArray<12>(5.1, 9.87);
  for(uint64_t i = 0; i < 12; i++) {
    EXPECT_EQ(reference.Uniform(5.1, 9.87), result2[i]);
  }
}

TEST(RandomTest, Gaus) {
  Random random;
  TRandom3 reference;

  random.SetSeed(42);
  reference.SetSeed(42);

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(), random.Gaus());
  }

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(i), random.Gaus(i));
  }

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(i, i+2), random.Gaus(i, i + 2));
  }
}

TEST_F(IOTest, Random) {
  Random random;
  TRandom3 reference;

  random.SetSeed(42);
  reference.SetSeed(42);

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Gaus(), random.Gaus());
  }

  Random* restored;
  BackupAndRestore(random, &restored);

  for(uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(reference.Uniform(i, i+2), random.Uniform(i, i + 2));
  }
}

}  // namespace bdm
