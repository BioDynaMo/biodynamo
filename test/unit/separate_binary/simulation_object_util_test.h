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

#ifndef UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_UTIL_TEST_H_
#define UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_UTIL_TEST_H_

#include "simulation_object_util.h"

#include <array>
#include <vector>

#include "compile_time_param.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "root_util.h"
#include "simulation_object.h"
#include "unit/test_sim_object.h"

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
  BDM_SIM_OBJECT_HEADER(MyCell, TestSimObject, 1, position_, diameter_);

 public:
  explicit MyCellExt(const std::array<double, 3>& pos) : position_{{pos}} {}

  MyCellExt() : position_{{1, 2, 3}} {}

  MostDerivedSoPtr Divide(double volume_ratio, double phi, double theta) {
    auto* ctxt = Simulation_t::GetActive()->GetExecCtxt();
    auto daughter = ctxt->template New<MostDerivedScalar>().GetSoPtr();
    ThisMD()->DivideImpl(daughter, volume_ratio, phi, theta);
    return daughter;
  }

  void DivideImpl(MostDerivedSoPtr daughter, double volume_ratio, double phi,
                  double theta) {
    daughter->SetPosition({5, 4, 3});
    diameter_[kIdx] = 1.123;
  }

  const std::array<double, 3>& GetPosition() const { return position_[kIdx]; }
  void SetPosition(const std::array<double, 3>& position) const {
    position_[kIdx] = position;
  }

  double GetDiameter() const { return diameter_[kIdx]; }

  void SetDiameter(double diameter) { diameter_[kIdx] = diameter; }

 protected:
  vec<std::array<double, 3>> position_;
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

  FRIEND_TEST(SimulationObjectUtilTest, NewEmptySoa);
  FRIEND_TEST(SimulationObjectUtilTest, SoaRef);
  FRIEND_TEST(SimulationObjectUtilTest, Soa_clear);
  FRIEND_TEST(SimulationObjectUtilTest, Soa_reserve);
};

// -----------------------------------------------------------------------------
// SOA object for IO test
BDM_SIM_OBJECT(TestObject, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(TestObject, SimulationObject, 1, id_);

 public:
  TestObjectExt() {}
  explicit TestObjectExt(int id) : id_(id) {}
  int GetId() const { return id_[kIdx]; }

 private:
  vec<int> id_;
};

BDM_SIM_OBJECT(TestThisMD, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(TestThisMD, SimulationObject, 0, foo_);

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

// }  // namespace simulation_object_util_test_internal

// has to be defined in namespace bdm
BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimObjectTypes = CTList<Neuron>;

  BDM_DEFAULT_CTPARAM_FOR(TestObject){};
  BDM_DEFAULT_CTPARAM_FOR(ContainerTestClass){};
  BDM_DEFAULT_CTPARAM_FOR(Neuron){};
  BDM_DEFAULT_CTPARAM_FOR(TestThisMD){};
  BDM_DEFAULT_CTPARAM_FOR(TestThisMDSubclass){};
  BDM_DEFAULT_CTPARAM_FOR(MyCell){};
};

// namespace simulation_object_util_test_internal {

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

#endif  // UNIT_SEPARATE_BINARY_SIMULATION_OBJECT_UTIL_TEST_H_
