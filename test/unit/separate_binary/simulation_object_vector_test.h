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

#ifndef UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_VECTOR_TEST_H_
#define UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_VECTOR_TEST_H_

#include <limits>

#include "gtest/gtest.h"

#include "backend.h"
#include "cell.h"
#include "compile_time_param.h"
#include "simulation_implementation.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "simulation_object_vector.h"
#include "unit/test_sim_object.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

BDM_SIM_OBJECT(A, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(A, TestSimObject, 1, id_);

 public:
  AExt() {}
  explicit AExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

BDM_SIM_OBJECT(B, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(B, TestSimObject, 1, id_);

 public:
  BExt() {}

  explicit BExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

}  // namespace simulation_object_vector_test_internal

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimObjectTypes = CTList<simulation_object_vector_test_internal::A,
                                simulation_object_vector_test_internal::B>;

  BDM_DEFAULT_CTPARAM_FOR(simulation_object_vector_test_internal::A){};
  BDM_DEFAULT_CTPARAM_FOR(simulation_object_vector_test_internal::B){};
};

namespace simulation_object_vector_test_internal {

inline void RunTest() {
  std::string sim_name("simulation_object_vector_test_RunInitializerTest");
  Simulation<> simulation(sim_name);
  auto* rm = simulation.GetResourceManager();

  ASSERT_EQ(2u, rm->NumberOfTypes());

  rm->push_back(A(3));
  rm->push_back(A(2));
  rm->push_back(A(1));

  rm->push_back(B(8));
  rm->push_back(B(9));

  SimulationObjectVector<int> vector;
  EXPECT_EQ(2u, vector.size());
  EXPECT_EQ(3u, vector.size(0));
  EXPECT_EQ(2u, vector.size(1));

  // values are not initialized
  vector[SoHandle(0, 0)] = 1;
  vector[SoHandle(0, 1)] = 2;
  vector[SoHandle(0, 2)] = 3;
  vector[SoHandle(1, 0)] = 4;
  vector[SoHandle(1, 1)] = 5;

  EXPECT_EQ(1, vector[SoHandle(0, 0)]);
  EXPECT_EQ(2, vector[SoHandle(0, 1)]);
  EXPECT_EQ(3, vector[SoHandle(0, 2)]);
  EXPECT_EQ(4, vector[SoHandle(1, 0)]);
  EXPECT_EQ(5, vector[SoHandle(1, 1)]);
}

}  // namespace simulation_object_vector_test_internal
}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_VECTOR_TEST_H_
