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

#ifndef UNIT_SEPARATE_BINARY_SO_POINTER_AOS_TEST_H_
#define UNIT_SEPARATE_BINARY_SO_POINTER_AOS_TEST_H_

#include <gtest/gtest.h>

#include "compile_time_param.h"
#include "simulation_backup.h"
#include "simulation_implementation.h"
#include "simulation_object.h"
#include "so_pointer.h"
#include "unit/io_test.h"
#include "unit/test_sim_object.h"

namespace bdm {

// SoPointer tests
/// This function is before the decleration of `SoPointerTestClass` to test
/// if the SoPointer implementation can deal with incomplete types
template <typename TSo, typename TBackend, typename TSimulation = Simulation<>>
void SoPointerTest(const TSo& so) {
  TSimulation::GetActive()->GetResourceManager()->push_back(so);

  SoPointer<TSo, TBackend> null_so_pointer;
  EXPECT_TRUE(null_so_pointer == nullptr);

  SoPointer<TSo, TBackend> so_ptr(so.GetUid());

  EXPECT_TRUE(so_ptr != nullptr);
  EXPECT_EQ(123u, so_ptr->GetId());

  so_ptr = nullptr;
  EXPECT_TRUE(so_ptr == nullptr);
}

BDM_SIM_OBJECT(SoPointerTestClass, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(SoPointerTestClass, TestSimObject, 1, my_so_ptr_, id_);

 public:
  SoPointerTestClassExt() {}
  SoPointerTestClassExt(uint64_t id) { id_[kIdx] = id; }

  uint64_t GetId() const { return id_[kIdx]; }
  void SetId(uint64_t id) { id_[kIdx] = id; }

  MostDerivedSoPtr GetMySoPtr() const { return my_so_ptr_[kIdx]; }
  void SetMySoPtr(MostDerivedSoPtr so_ptr) { my_so_ptr_[kIdx] = so_ptr; }

  vec<MostDerivedSoPtr> my_so_ptr_ = {{}};

 private:
  vec<uint64_t> id_;
};

// has to be defined in namespace bdm
BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimulationBackend = Scalar;
  using SimObjectTypes = CTList<SoPointerTestClass>;
  BDM_DEFAULT_CTPARAM_FOR(SoPointerTestClass){};
};

inline void IOTestSoPointerRmContainerAos(Simulation<>* sim) {
  auto* rm = sim->GetResourceManager();
  rm->push_back(SoPointerTestClass(123));
  SoPointerTestClass so2(456);
  rm->push_back(so2);

  SoPointer<SoPointerTestClass, Scalar> soptr(so2.GetUid());
  SoPointer<SoPointerTestClass, Scalar>* restored;

  BackupAndRestore(soptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(456u, (*restored)->GetId());
}

inline void IOTestSoPointerNullptr() {
  SoPointer<SoPointerTestClass, Scalar> null_so_pointer;
  SoPointer<SoPointerTestClass, Scalar>* restored = nullptr;

  BackupAndRestore(null_so_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SO_POINTER_AOS_TEST_H_
