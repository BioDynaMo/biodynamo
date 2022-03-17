// -----------------------------------------------------------------------------
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

#include "core/util/type.h"
#include <string>
#include <typeinfo>
#include "core/agent/cell.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(TypeTernaryOperatorTest, True) {
  type_ternary_operator<true, int, real>::type data;
  EXPECT_EQ(std::string("i"), typeid(data).name());
}

TEST(TypeTernaryOperatorTest, False) {
  type_ternary_operator<false, int, real>::type data;
  EXPECT_EQ(typeid(real).name(), typeid(data).name());
}

TEST(TestRawType, All) {
  {
    bool result = std::is_same<int, raw_type<int*>>::value;
    EXPECT_TRUE(result);
  }
  {
    bool result = std::is_same<int, raw_type<int&>>::value;
    EXPECT_TRUE(result);
  }
  {
    bool result = std::is_same<int, raw_type<int&&>>::value;
    EXPECT_TRUE(result);
  }
  {
    bool result = std::is_same<int, raw_type<int*&>>::value;
    EXPECT_TRUE(result);
  }
}

}  // namespace bdm
