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

#ifndef UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_AOS_TEST_H_
#define UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_AOS_TEST_H_

#include "unit/separate_binary/resource_manager_test_common.h"
#include "transactional_vector.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimObjectTypes = CTList<A, B>;
  using SimulationBackend = Scalar;
  BDM_DEFAULT_CTPARAM_FOR(A){};
  BDM_DEFAULT_CTPARAM_FOR(B){};
};

inline void RunIOTest() {
  const double kEpsilon = abs_error<double>::value;
  ResourceManager<> rm;
  remove(ROOTFILE);

  // setup
  auto a_vector = rm.Get<A>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(A(12));
  a_vector->push_back(A(34));
  a_vector->push_back(A(42));

  auto b_vector = rm.Get<B>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(B(3.14));
  b_vector->push_back(B(6.28));

  DiffusionGrid* dgrid_1 = new DiffusionGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid(1, "Natrium", 0.2, 0.1, 1);
  rm.AddDiffusionGrid(dgrid_1);
  rm.AddDiffusionGrid(dgrid_2);

  // backup
  WritePersistentObject(ROOTFILE, "rm", rm, "new");

  rm.Clear();

  // restore
  ResourceManager<>* restored_rm = nullptr;
  GetPersistentObject(ROOTFILE, "rm", restored_rm);

  // validate
  EXPECT_EQ(5u, restored_rm->GetNumSimObjects());

  ASSERT_EQ(3u, restored_rm->Get<A>()->size());
  EXPECT_EQ(12, (*restored_rm->Get<A>())[0].GetData());
  EXPECT_EQ(34, (*restored_rm->Get<A>())[1].GetData());
  EXPECT_EQ(42, (*restored_rm->Get<A>())[2].GetData());

  ASSERT_EQ(2u, restored_rm->Get<B>()->size());
  EXPECT_NEAR(3.14, (*restored_rm->Get<B>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*restored_rm->Get<B>())[1].GetData(), kEpsilon);

  EXPECT_EQ(0, restored_rm->GetDiffusionGrid(0)->GetSubstanceId());
  EXPECT_EQ(1, restored_rm->GetDiffusionGrid(1)->GetSubstanceId());
  EXPECT_EQ("Kalium", restored_rm->GetDiffusionGrid(0)->GetSubstanceName());
  EXPECT_EQ("Natrium", restored_rm->GetDiffusionGrid(1)->GetSubstanceName());
  EXPECT_EQ(0.6,
            restored_rm->GetDiffusionGrid(0)->GetDiffusionCoefficients()[0]);
  EXPECT_EQ(0.8,
            restored_rm->GetDiffusionGrid(1)->GetDiffusionCoefficients()[0]);

  delete restored_rm;

  remove(ROOTFILE);
}

}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_RESOURCE_MANAGER_AOS_TEST_H_
