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

#ifndef UNIT_SIMULATION_OBJECT_TEST_H_
#define UNIT_SIMULATION_OBJECT_TEST_H_

#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "unit/test_util.h"

namespace bdm {
namespace simulation_object_test_internal {

template <typename TBackend = Soa>
struct CTParam {
  template <typename TTBackend>
  using Self = CTParam<TTBackend>;
  using Backend = TBackend;
};

template <template <typename, typename> class>
struct TestCapsule;

template <>
struct TestCapsule<SimulationObject> {
  template <typename T1, typename T2>
  using type = SimulationObject<T1, T2>;
};

inline void RunPushBackAndClearTest() {
  SimulationObject<CTParam<>, TestCapsule<SimulationObject>> soa;
  // call clear, because creating a SOA object with default constructor will
  // already have one element inside
  soa.clear();
  EXPECT_EQ(0u, soa.size());
  SimulationObject<CTParam<Scalar>, TestCapsule<SimulationObject>> so;
  soa.push_back(so);
  soa.push_back(so);
  EXPECT_EQ(2u, soa.size());
  soa.clear();
  EXPECT_EQ(0u, soa.size());
}

}  // namespace simulation_object_test_internal
}  // namespace bdm

#endif  // UNIT_SIMULATION_OBJECT_TEST_H_
