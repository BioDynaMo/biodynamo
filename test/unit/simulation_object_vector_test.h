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

#ifndef UNIT_SIMULATION_OBJECT_VECTOR_TEST_H_
#define UNIT_SIMULATION_OBJECT_VECTOR_TEST_H_

#include <limits>

#include "gtest/gtest.h"

#include "backend.h"
#include "cell.h"
#include "compile_time_param.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "simulation_object_vector.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

template <typename TBackend = Soa>
struct CompileTimeParam1;

BDM_SIM_OBJECT_TEST(A, SimulationObject, CompileTimeParam1) {
  BDM_SIM_OBJECT_HEADER(AExt, 1, id_);

 public:
  AExt() {}
  explicit AExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

BDM_SIM_OBJECT_TEST(B, SimulationObject, CompileTimeParam1) {
  BDM_SIM_OBJECT_HEADER(BExt, 1, id_);

 public:
  BExt() {}

  explicit BExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

template <typename TBackend>
struct CompileTimeParam1 {
  template <typename TTBackend>
  using Self = CompileTimeParam1<TTBackend>;
  using Backend = TBackend;

  using SimulationBackend = Soa;
  using AtomicTypes = VariadicTypedef<A, B>;
};

inline void RunTest() {
  std::string sim_name("simulation_object_vector_test_RunInitializerTest");
  using BdmSim = BdmSim<CompileTimeParam1<Soa>>;
  BdmSim simulation(sim_name);
  auto* rm = simulation.GetRm();

  ASSERT_EQ(2u, rm->NumberOfTypes());

  auto as = rm->Get<A>();
  as->push_back(A(3));
  as->push_back(A(2));
  as->push_back(A(1));

  auto bs = rm->Get<B>();
  bs->push_back(B(8));
  bs->push_back(B(9));

  SimulationObjectVector<int, BdmSim> vector;
  EXPECT_EQ(0, vector[SoHandle(0, 0)]);
  EXPECT_EQ(0, vector[SoHandle(0, 1)]);
  EXPECT_EQ(0, vector[SoHandle(0, 2)]);
  EXPECT_EQ(0, vector[SoHandle(1, 0)]);
  EXPECT_EQ(0, vector[SoHandle(1, 1)]);

  vector[SoHandle(1, 0)] = 7;
  EXPECT_EQ(7, vector[SoHandle(1, 0)]);

  vector[SoHandle(0, 2)] = 5;
  EXPECT_EQ(5, vector[SoHandle(0, 2)]);
}

template <typename TBackend = Soa>
struct DefaultCompileTimeParam1 {
  template <typename TTBackend>
  using Self = CompileTimeParam<TTBackend>;
  using Backend = TBackend;

  /// Defines backend used in ResourceManager
  using SimulationBackend = Soa;
  using BiologyModules = Variant<NullBiologyModule>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

// Tests if SimulationObjectVector::Initialize does indeed initialize the
// object correctly (i.e. set the data_ member to the default values)
inline void RunInitializeTest() {
  std::string sim_name("simulation_object_vector_test_RunInitializerTest");
  using BdmSim = BdmSim<CompileTimeParam1<Soa>>;
  BdmSim simulation(sim_name);
  auto* rm = simulation.GetRm();

  ASSERT_EQ(2u, rm->NumberOfTypes());

  // Add some simulation objects, which shall be of type 0
  auto as = rm->Get<A>();
  as->push_back(A(3));
  as->push_back(A(2));
  as->push_back(A(1));

  // Add some simulation objects, which shall be of type 1
  auto bs = rm->Get<B>();
  bs->push_back(B(8));
  bs->push_back(B(9));

  // The exected initial values for SoHandle objects
  auto max16 = std::numeric_limits<uint16_t>::max();
  auto max32 = std::numeric_limits<uint32_t>::max();

  // Check if the initial state is obtained after the push backs
  SimulationObjectVector<SoHandle, BdmSim> vector;
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 0)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 1)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 2)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(1, 0)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(1, 1)]);

  // Assign a value to object 0 of simulation type 1
  vector[SoHandle(1, 0)] = SoHandle(1, 0);
  EXPECT_EQ(SoHandle(1, 0), vector[SoHandle(1, 0)]);

  // Assign a value to object 2 of simulation type 0
  vector[SoHandle(0, 2)] = SoHandle(0, 2);
  EXPECT_EQ(SoHandle(0, 2), vector[SoHandle(0, 2)]);

  // Initialize should reset vector to the default values
  vector.Initialize();

  // Check if Initialize indeed reset the values to the defaults
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 0)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 1)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(0, 2)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(1, 0)]);
  EXPECT_EQ(SoHandle(max16, max32), vector[SoHandle(1, 1)]);
}

}  // namespace simulation_object_vector_test_internal
}  // namespace bdm

#endif  // UNIT_SIMULATION_OBJECT_VECTOR_TEST_H_
