// -----------------------------------------------------------------------------
//
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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
#include "core/real_t.h"
#include <gtest/gtest.h>

namespace bdm {

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, Default) {
  auto f = L2F([](int i, real_t d) { return i * d; });
  auto result = f(3, 2.0);
  EXPECT_NEAR(6, result, 1e-5);
  bool is_base =
      std::is_base_of<Functor<real_t, int, real_t>, decltype(f)>::value;
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

// -----------------------------------------------------------------------------
TEST(Lambda2Functor, FunctionParam) {
  auto f = L2F([](int i) { return ++i; });
  int i = 0;
  EXPECT_EQ(1, f(i));
  bool is_base = std::is_base_of<Functor<int, int>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

TEST(Lambda2Functor, FunctionParamLValueRef) {
  auto f = L2F([](int& i) { ++i; });
  int i = 0;
  f(i);
  EXPECT_EQ(1, i);
  bool is_base = std::is_base_of<Functor<void, int&>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

TEST(Lambda2Functor, FunctionParamRValueRef) {
  auto f = L2F([](int&& i) { ++i; });
  int i = 0;
  f(std::move(i));
  EXPECT_EQ(1, i);
  bool is_base = std::is_base_of<Functor<void, int&&>, decltype(f)>::value;
  EXPECT_TRUE(is_base);
}

}  // namespace bdm
