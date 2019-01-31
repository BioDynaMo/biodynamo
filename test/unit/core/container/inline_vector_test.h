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

#ifndef UNIT_CORE_CONTAINER_INLINE_VECTOR_TEST_H_
#define UNIT_CORE_CONTAINER_INLINE_VECTOR_TEST_H_

#include <vector>
#include "gtest/gtest.h"

#include "core/container/inline_vector.h"
#include "core/util/io.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace inline_vector_test_internal {

// IO related code must be in a header file.
inline void RunIOTest() {
  remove(ROOTFILE);

  InlineVector<int, 8> neighbor;
  for (int i = 0; i < 15; i++) {
    neighbor.push_back(i);
  }

  OneElementArray<InlineVector<int, 8>> aoi_scalar(neighbor);
  std::vector<InlineVector<int, 8>> aoi_vector;
  for (int i = 0; i < 4; i++) {
    aoi_vector.push_back(neighbor);
  }

  WritePersistentObject(ROOTFILE, "InlineVector", neighbor, "new");
  WritePersistentObject(ROOTFILE, "S_InlineVector", aoi_scalar, "update");
  WritePersistentObject(ROOTFILE, "V_InlineVector", aoi_vector, "update");

  InlineVector<int, 8>* neighbor_r = nullptr;
  OneElementArray<InlineVector<int, 8>>* aoi_scalar_r = nullptr;
  std::vector<InlineVector<int, 8>>* aoi_vector_r = nullptr;

  GetPersistentObject(ROOTFILE, "InlineVector", neighbor_r);
  GetPersistentObject(ROOTFILE, "S_InlineVector", aoi_scalar_r);
  GetPersistentObject(ROOTFILE, "V_InlineVector", aoi_vector_r);

  EXPECT_EQ(neighbor.size(), neighbor_r->size());

  if (!(neighbor == (*neighbor_r))) {
    FAIL();
  }

  if (!(aoi_scalar[0] == (*aoi_scalar_r)[0])) {
    FAIL();
  }

  for (size_t i = 0; i < aoi_vector.size(); i++) {
    if (!(aoi_vector[i] == (*aoi_vector_r)[i])) {
      FAIL();
    }
  }

  delete neighbor_r;
  delete aoi_scalar_r;
  delete aoi_vector_r;

  remove(ROOTFILE);
}

}  // namespace inline_vector_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_CONTAINER_INLINE_VECTOR_TEST_H_
