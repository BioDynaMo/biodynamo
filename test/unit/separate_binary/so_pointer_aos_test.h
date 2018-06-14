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
#include "simulation_object.h"
#include "so_pointer.h"
#include "unit/io_test.h"
#include "bdm_imp.h"

namespace bdm {
namespace so_pointer_aos_test_internal {

// SoPointer tests
/// This function is before the decleration of `SoPointerTestClass` to test
/// if the SoPointer implementation can deal with incomplete types
template <typename T, typename TBackend>
void SoPointerTest(T* sim_objects) {
  using SO = typename T::value_type;

  SoPointer<SO, TBackend> null_so_pointer;
  EXPECT_TRUE(null_so_pointer == nullptr);

  SoPointer<SO, TBackend> so_ptr(sim_objects, 0);

  EXPECT_FALSE(so_ptr == nullptr);
  EXPECT_EQ(123u, so_ptr->GetId());

  so_ptr = nullptr;
  EXPECT_TRUE(so_ptr == nullptr);
}

BDM_SIM_OBJECT(SoPointerTestClass, bdm::SimulationObject) {
  BDM_SIM_OBJECT_HEADER(SoPointerTestClassExt, 1, my_so_ptr_, id_);

 public:
  SoPointerTestClassExt() {}
  SoPointerTestClassExt(uint64_t id) { id_[kIdx] = id; }

  uint64_t GetId() const { return id_[kIdx]; }
  void SetId(uint64_t id) { id_[kIdx] = id; }

  MostDerivedSoPtr GetMySoPtr() const { return my_so_ptr_[kIdx]; }
  void SetMySoPtr(MostDerivedSoPtr so_ptr) { my_so_ptr_[kIdx] = so_ptr; }

  vec<MostDerivedSoPtr> my_so_ptr_ = {{}};

  // FIXME
  std::array<double, 3> GetPosition() const { return {0, 0, 0}; };
  void SetPosition(const std::array<double, 3>&) {}
  void ApplyDisplacement(const std::array<double, 3>&) {}
  template <typename TGrid>
  std::array<double, 3> CalculateDisplacement(TGrid * grid,
                                              double squared_radius) { return {0, 0, 0}; };
  void RunBiologyModules() {}
  void SetBoxIdx(uint64_t) {}
  double GetDiameter() {}
  static std::set<std::string> GetRequiredVisDataMembers() { return {"diameter_", "position_"}; };
  static constexpr Shape GetShape() { return Shape::kSphere; }

 private:
  vec<uint64_t> id_;
};

}  // namespace so_pointer_aos_test_internal

// has to be defined in namespace bdm
template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {
  using SimulationBackend = Scalar;
  using AtomicTypes =
      VariadicTypedef<so_pointer_aos_test_internal::SoPointerTestClass>;
};

namespace so_pointer_aos_test_internal {

inline void IOTestSoPointerRmContainerAos() {
  BdmSim<> simulation("IOTestSoPointerRmContainerAos");
  auto* rm = simulation.GetRm();
  // TODO(lukas) Remove after https://trello.com/c/sKoOTgJM has been resolved
  rm->Get<SoPointerTestClass>()->reserve(2);
  auto&& so1 = rm->New<SoPointerTestClass>(123);
  auto&& so2 = rm->New<SoPointerTestClass>(456);

  auto soptr = so1.GetSoPtr();
  EXPECT_EQ(0u, soptr.GetElementIdx());
  so2.SetMySoPtr(soptr);

  SimulationBackup backup(IOTest::kRootFile, "");
  backup.Backup(1);

  // to see if objects are restorec correctly, clear RessourceManager
  rm->Clear();

  SimulationBackup restore("", IOTest::kRootFile);
  restore.Restore();

  EXPECT_EQ(rm, simulation.GetRm());

  auto* restored_sim_objects = rm->Get<SoPointerTestClass>();
  EXPECT_EQ(123u, (*restored_sim_objects)[1].GetMySoPtr()->GetId());
  // change id of first element
  (*restored_sim_objects)[0].SetId(987);
  // id should have changed
  EXPECT_EQ(987u, (*restored_sim_objects)[1].GetMySoPtr()->GetId());
}

}  // namespace so_pointer_aos_test_internal

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SO_POINTER_AOS_TEST_H_
