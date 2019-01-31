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

#ifndef UNIT_SEPARATE_BINARY_SIM_OBJECT_UTIL_TEST_H_
#define UNIT_SEPARATE_BINARY_SIM_OBJECT_UTIL_TEST_H_

#include <array>
#include <vector>

#include "core/param/compile_time_param.h"
#include "core/sim_object/sim_object.h"
#include "core/util/io.h"
#include "core/util/root.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_sim_object.h"

#define ROOTFILE "bdmFile.root"

#define FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test

namespace bdm {
// namespace simulation_object_util_test_internal {

BDM_SIM_OBJECT(ContainerTestClass, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(ContainerTestClass, TestSimObject, 1, dm1_, dm2_);

 public:
  ContainerTestClassExt() {}
  ContainerTestClassExt(int i, double d) {
    dm1_[kIdx] = i;
    dm2_[kIdx] = d;
  }

  const int GetDm1() const { return dm1_[kIdx]; }
  const double GetDm2() const { return dm2_[kIdx]; }

  const vec<int>& GetVecDm1() const { return dm1_; }
  const vec<double>& GetVecDm2() const { return dm2_; }

 private:
  vec<int> dm1_;
  vec<double> dm2_;
};

BDM_SIM_OBJECT(MyCell, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(MyCell, TestSimObject, 1, diameter_);

 public:
  explicit MyCellExt(const std::array<double, 3>& pos) : Base(pos) {}

  MyCellExt() : Base({1, 2, 3}) {}

  MostDerivedSoPtr Divide(double volume_ratio, double phi, double theta) {
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    auto daughter = ctxt->template New<MostDerivedScalar>().GetSoPtr();
    ThisMD()->DivideImpl(daughter, volume_ratio, phi, theta);
    return daughter;
  }

  void DivideImpl(MostDerivedSoPtr daughter, double volume_ratio, double phi,
                  double theta) {
    daughter->SetPosition({5, 4, 3});
    diameter_[kIdx] = 1.123;
  }

  double GetDiameter() const { return diameter_[kIdx]; }

  void SetDiameter(double diameter) { diameter_[kIdx] = diameter; }

 protected:
  vec<double> diameter_ = {6.28};
};

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {
 public:
  explicit Neurite(TRootIOCtor* io_ctor) {}
  Neurite() : id_(0) {}
  explicit Neurite(std::size_t id) : id_(id) {}
  virtual ~Neurite() {}
  std::size_t id_;

 private:
  BDM_CLASS_DEF(Neurite, 1);  // NOLINT
};

// add Neurites to BaseCell
BDM_SIM_OBJECT(Neuron, MyCell) {
  BDM_SIM_OBJECT_HEADER(Neuron, MyCell, 1, neurites_);

 public:
  template <class... A>
  explicit NeuronExt(const std::vector<Neurite>& neurites, const A&... a)
      : Base(a...), neurites_{{neurites}} {}

  NeuronExt() = default;

  void DivideImpl(MostDerivedSoPtr daughter, double volume_ratio, double phi,
                  double theta) {
    daughter->neurites_[daughter->kIdx].push_back(Neurite(987));
    Base::DivideImpl(daughter, volume_ratio, phi, theta);
  }

  const std::vector<Neurite>& GetNeurites() const { return neurites_[kIdx]; }

 private:
  vec<std::vector<Neurite>> neurites_ = {{}};

  FRIEND_TEST(SimObjectUtilTest, NewEmptySoa);
  FRIEND_TEST(SimObjectUtilTest, SoaRef);
  FRIEND_TEST(SimObjectUtilTest, Soa_clear);
  FRIEND_TEST(SimObjectUtilTest, Soa_reserve);
};

// -----------------------------------------------------------------------------
// SOA object for IO test
BDM_SIM_OBJECT(TestObject, SimObject) {
  BDM_SIM_OBJECT_HEADER(TestObject, SimObject, 1, id_);

 public:
  TestObjectExt() {}
  explicit TestObjectExt(int id) : id_(id) {}
  int GetId() const { return id_[kIdx]; }

 private:
  vec<int> id_;
};

BDM_SIM_OBJECT(TestThisMD, SimObject) {
  BDM_SIM_OBJECT_HEADER(TestThisMD, SimObject, 0, foo_);

 public:
  TestThisMDExt() {}

  int AnotherFunction() { return 123; }

  int SomeFunction() { return ThisMD()->AnotherFunction(); }

  vec<int> foo_;
};

BDM_SIM_OBJECT(TestThisMDSubclass, TestThisMD) {
  BDM_SIM_OBJECT_HEADER(TestThisMDSubclass, TestThisMD, 0, foo_);

 public:
  TestThisMDSubclassExt() {}

  int AnotherFunction() { return 321; }

  vec<int> foo_;
};

inline void RunSoaIOTest() {
  remove(ROOTFILE);

  auto objects = SoaTestObject::NewEmptySoa();
  for (size_t i = 0; i < 10; i++) {
    objects.push_back(TestObject(i));
  }

  // write to root file
  WritePersistentObject(ROOTFILE, "objects", objects, "new");

  // read back
  SoaTestObject* restored_objects = nullptr;
  GetPersistentObject(ROOTFILE, "objects", restored_objects);

  // validate
  EXPECT_EQ(10u, restored_objects->size());
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*restored_objects)[i].GetId());
  }

  // delete root file
  remove(ROOTFILE);
}

// }  // namespace simulation_object_util_test_internal
}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_SIM_OBJECT_UTIL_TEST_H_
