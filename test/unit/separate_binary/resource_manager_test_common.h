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

#ifndef UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_
#define UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_

#include "resource_manager.h"

#include <vector>
#include "backend.h"
#include "compile_time_list.h"
#include "compile_time_param.h"
#include "io_util.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "type_util.h"
#include "unit/test_sim_object.h"
#include "unit/test_util.h"

namespace bdm {

BDM_SIM_OBJECT(A, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(A, TestSimObject, 1, data_);

 public:
  AExt() {}  // for ROOT I/O
  explicit AExt(int data) { data_[kIdx] = data; }

  int GetData() const { return data_[kIdx]; }
  void SetData(int data) { data_[kIdx] = data; }

  vec<int> data_;
};

BDM_SIM_OBJECT(B, TestSimObject) {
  BDM_SIM_OBJECT_HEADER(B, TestSimObject, 1, data_);

 public:
  BExt() {}  // for ROOT I/O
  explicit BExt(double data) { data_[kIdx] = data; }

  double GetData() const { return data_[kIdx]; }
  void SetData(double data) { data_[kIdx] = data; }

  vec<double> data_;
};

/// Create ResourceManager with two types, use Get function to obtain container
/// of the specified type, push_back values and check if they have correctly
/// been added inside the ResourceManager
template <typename TRm, typename TA, typename TB>
inline void RunGetTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  // template specifier needed because A is dependant type
  EXPECT_EQ(0u, rm.template Get<TA>()->size());
  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  EXPECT_EQ(2u, rm.template Get<TA>()->size());
  EXPECT_EQ(12, (*rm.template Get<TA>())[0].GetData());
  EXPECT_EQ(34, (*rm.template Get<TA>())[1].GetData());

  EXPECT_EQ(0u, rm.template Get<TB>()->size());
  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  EXPECT_EQ(2u, rm.template Get<TB>()->size());
  EXPECT_NEAR(3.14, (*rm.template Get<TB>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*rm.template Get<TB>())[1].GetData(), kEpsilon);
}

template <typename TRm, typename TA, typename TB>
inline void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;
  rm.Clear();

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  rm.ApplyOnElement(SoHandle(0, 1),
                    [](auto&& element) { EXPECT_EQ(34, element.GetData()); });

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  rm.ApplyOnElement(SoHandle(1, 0), [&](auto&& element) {
    EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
  });
}

template <typename TRm, typename TA, typename TB>
void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  size_t counter = 0;
  rm.ApplyOnAllElements([&](auto&& element, SoHandle&& handle) {  // NOLINT
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(12, element.GetData());
        break;
      case 2:
        EXPECT_EQ(34, element.GetData());
        break;
      case 3:
        EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
        break;
      case 4:
        EXPECT_NEAR(6.28, element.GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(4u, counter);
}

template <typename TRm, typename TA, typename TB>
void RunGetNumSimObjects() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));
  rm.push_back(TAScalar(59));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));

  EXPECT_EQ(5u, rm.GetNumSimObjects());
}

// This test uses Cells since SoaA, SoaB are strippted down simulatio objects
// and are themselves not thread safe.
template <typename TRm, typename TB>
void RunApplyOnAllElementsParallelTest() {
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  rm.push_back(TBScalar(9.42));

  rm.ApplyOnAllElementsParallel([](auto&& element, SoHandle handle) {
    const double kEpsilon = abs_error<double>::value;
    if (handle == SoHandle(1, 0)) {
      EXPECT_EQ(3.14, element.GetData());
    } else if (handle == SoHandle(1, 1)) {
      EXPECT_EQ(6.28, element.GetData());
    } else if (handle == SoHandle(1, 2)) {
      EXPECT_NEAR(9.42, element.GetData(), kEpsilon);
    } else {
      FAIL();
    }
  });
}

template <typename TRm, typename TA, typename TB>
void RunRemoveAndContainsTest() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  TAScalar a0(12);
  auto a0_uid = a0.GetUid();
  rm.push_back(a0);

  TAScalar a1(34);
  auto a1_uid = a1.GetUid();
  rm.push_back(a1);

  TAScalar a2(59);
  auto a2_uid = a2.GetUid();
  rm.push_back(a2);

  TBScalar b0(3.14);
  auto b0_uid = b0.GetUid();
  rm.push_back(b0);

  TBScalar b1(6.28);
  auto b1_uid = b1.GetUid();
  rm.push_back(b1);

  EXPECT_TRUE(rm.Contains(a0_uid));
  EXPECT_TRUE(rm.Contains(a1_uid));
  EXPECT_TRUE(rm.Contains(a2_uid));
  EXPECT_TRUE(rm.Contains(b0_uid));
  EXPECT_TRUE(rm.Contains(b1_uid));

  rm.Remove(a0_uid);
  rm.Remove(a1_uid);
  rm.Remove(a2_uid);
  rm.Remove(b0_uid);
  rm.Remove(b1_uid);

  EXPECT_FALSE(rm.Contains(a0_uid));
  EXPECT_FALSE(rm.Contains(a1_uid));
  EXPECT_FALSE(rm.Contains(a2_uid));
  EXPECT_FALSE(rm.Contains(b0_uid));
  EXPECT_FALSE(rm.Contains(b1_uid));

  EXPECT_EQ(0u, rm.GetNumSimObjects());
}

template <typename TRm, typename TA, typename TB>
void RunClearTest() {
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  TAScalar a0(12);
  auto a0_uid = a0.GetUid();
  rm.push_back(a0);

  TAScalar a1(34);
  auto a1_uid = a1.GetUid();
  rm.push_back(a1);

  TAScalar a2(59);
  auto a2_uid = a2.GetUid();
  rm.push_back(a2);

  TBScalar b0(3.14);
  auto b0_uid = b0.GetUid();
  rm.push_back(b0);

  TBScalar b1(6.28);
  auto b1_uid = b1.GetUid();
  rm.push_back(b1);

  EXPECT_TRUE(rm.Contains(a0_uid));
  EXPECT_TRUE(rm.Contains(a1_uid));
  EXPECT_TRUE(rm.Contains(a2_uid));
  EXPECT_TRUE(rm.Contains(b0_uid));
  EXPECT_TRUE(rm.Contains(b1_uid));

  rm.Clear();

  EXPECT_FALSE(rm.Contains(a0_uid));
  EXPECT_FALSE(rm.Contains(a1_uid));
  EXPECT_FALSE(rm.Contains(a2_uid));
  EXPECT_FALSE(rm.Contains(b0_uid));
  EXPECT_FALSE(rm.Contains(b1_uid));

  EXPECT_EQ(0u, rm.GetNumSimObjects());
}

template <typename TRm, typename TA, typename TB>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));
  size_t counter = 0;
  rm.ApplyOnAllTypes([&](auto* container, uint16_t type_idx) {
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(1u, container->size());
        EXPECT_EQ(12, (*container)[0].GetData());
        break;
      case 2:
        EXPECT_EQ(2u, container->size());
        EXPECT_NEAR(3.14, (*container)[0].GetData(), kEpsilon);
        EXPECT_NEAR(6.28, (*container)[1].GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(2u, counter);
}

template <typename TRm, typename TA, typename TB>
void RunGetTypeIndexTest() {
  TRm rm;
  EXPECT_EQ(0u, TRm::template GetTypeIndex<TA>());
  EXPECT_EQ(1u, TRm::template GetTypeIndex<TB>());
}

template <typename TRm, typename TA, typename TB>
void RunPushBackTest() {
  const double kEpsilon = abs_error<double>::value;
  using TAScalar = typename TA::template Self<Scalar>;
  using TBScalar = typename TB::template Self<Scalar>;
  TRm rm;

  rm.push_back(TAScalar(12));
  rm.push_back(TAScalar(34));

  rm.push_back(TBScalar(3.14));
  rm.push_back(TBScalar(6.28));

  rm.push_back(TAScalar(87));

  const auto* as = rm.template Get<TA>();
  const auto* bs = rm.template Get<TB>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);
}

template <typename TA, typename TB, typename TRm = ResourceManager<>>
void RunGetSimObjectTest() {
  TRm rm;
  auto current_uid = SoUidGenerator::Get()->NewSoUid();
  rm.push_back(TA(0));
  rm.push_back(TA(1));
  rm.push_back(TB(2));
  rm.push_back(TB(3));

  const double kEpsilon = abs_error<double>::value;
  EXPECT_EQ(0, rm.template GetSimObject<TA>(SoHandle(0, 0)).GetData());
  EXPECT_EQ(1, rm.template GetSimObject<TA>(SoHandle(0, 1)).GetData());
  EXPECT_NEAR(2, rm.template GetSimObject<TB>(SoHandle(1, 0)).GetData(), kEpsilon);
  EXPECT_NEAR(3, rm.template GetSimObject<TB>(SoHandle(1, 1)).GetData(), kEpsilon);

  EXPECT_EQ(0, rm.template GetSimObject<TA>(current_uid +1).GetData());
  EXPECT_EQ(1, rm.template GetSimObject<TA>(current_uid + 2).GetData());
  EXPECT_NEAR(2, rm.template GetSimObject<TB>(current_uid + 3).GetData(), kEpsilon);
  EXPECT_NEAR(3, rm.template GetSimObject<TB>(current_uid + 4).GetData(), kEpsilon);
}

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_TEST_COMMON_H_
