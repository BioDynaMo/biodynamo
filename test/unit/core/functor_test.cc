// -----------------------------------------------------------------------------
//
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/functor.h"
#include <gtest/gtest.h>

namespace bdm {

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, Default) {
  auto f = L2F([](int i, double d) { return i * d; });
  auto result = f(3, 2.0);
  EXPECT_NEAR(6, result, 1e-5);
  bool is_base =
      std::is_base_of<Functor<double, int, double>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, CaptureByReference) {
  int i = 0;
  auto f = L2F([&]() { return ++i; });
  EXPECT_EQ(1, f());
  EXPECT_EQ(1, i);
  bool is_base = std::is_base_of<Functor<int>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, CaptureByValue) {
  int i = 2;
  auto f = L2F([=]() { return i; });
  EXPECT_EQ(2, f());
  EXPECT_EQ(2, i);
  bool is_base = std::is_base_of<Functor<int>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, Mutable) {
  int i = 1;
  auto f = L2F([=]() mutable { return ++i; });
  EXPECT_EQ(2, f());
  EXPECT_EQ(1, i);
  bool is_base = std::is_base_of<Functor<int>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, NoParamRetVoid) {
  int i = 0;
  auto f = L2F([&]() { ++i; });
  f();
  EXPECT_EQ(1, i);
  bool is_base = std::is_base_of<Functor<void>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

}  // namespace bdm
