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

#ifndef UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_
#define UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_

#include <gtest/gtest.h>

#include "core/param/compile_time_param.h"
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_pointer.h"
#include "core/simulation_backup.h"
#include "core/simulation_implementation.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {
namespace so_pointer_test_internal {

// SoPointer tests
/// This function is before the decleration of `SoPointerTestClass` to test
/// if the SoPointer implementation can deal with incomplete types
template <typename TSo, typename TBackend, typename TSimulation = Simulation>
void SoPointerTest(const TSo& so) {
  Simulation::GetActive()->GetResourceManager()->push_back(so);

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

}  // namespace so_pointer_test_internal

// has to be defined in namespace bdm
BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimObjectTypes = CTList<so_pointer_test_internal::SoPointerTestClass>;

  BDM_DEFAULT_CTPARAM_FOR(so_pointer_test_internal::SoPointerTestClass) {
    using BiologyModules = CTList<NullBiologyModule>;
  };
};

namespace so_pointer_test_internal {

inline void RunIOTest(Simulation* sim) {
  auto* rm = sim->GetResourceManager();
  rm->push_back(SoPointerTestClass(123));
  SoPointerTestClass so2(456);
  rm->push_back(so2);

  SoPointer<SoPointerTestClass, Soa> soptr(so2.GetUid());
  SoPointer<SoPointerTestClass, Soa>* restored;

  BackupAndRestore(soptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(456u, (*restored)->GetId());
}

inline void IOTestSoPointerNullptr() {
  SoPointer<SoaSoPointerTestClass, Soa> null_so_pointer;
  SoPointer<SoaSoPointerTestClass, Soa>* restored = nullptr;

  BackupAndRestore(null_so_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

}  // namespace so_pointer_test_internal

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SO_POINTER_TEST_H_
