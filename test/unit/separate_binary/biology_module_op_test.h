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

#ifndef UNIT_BIOLOGY_MODULE_OP_TEST_H_
#define UNIT_BIOLOGY_MODULE_OP_TEST_H_

#include "biology_module_op.h"
#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_param.h"
#include "transactional_vector.h"
#include "unit/test_util.h"

namespace bdm {
namespace biology_module_op_test_internal {

using std::size_t;

struct GrowthModule : public BaseBiologyModule {
  double growth_rate_;

  GrowthModule() : growth_rate_(0.5) {}
  explicit GrowthModule(double growth_rate) : growth_rate_(growth_rate) {}

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  BDM_CLASS_DEF_NV(GrowthModule, 1);
};

}  // namespace biology_module_op_test_internal

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  BDM_CTPARAM_FOR(bdm, Cell) {
    using BiologyModules =
        CTList<biology_module_op_test_internal::GrowthModule>;
  };
};

namespace biology_module_op_test_internal {

template <typename T>
inline void RunTest(T* cells) {
  CellTest<CompileTimeParam<Scalar>> cell_1(12);
  cell_1.AddBiologyModule(GrowthModule(2));

  CellTest<CompileTimeParam<Scalar>> cell_2(34);
  cell_2.AddBiologyModule(GrowthModule(3));

  cells->push_back(cell_1);
  cells->push_back(cell_2);

  BiologyModuleOp op;
  op(cells, 0);

  EXPECT_EQ(2u, cells->size());
  EXPECT_NEAR(14, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(37, (*cells)[1].GetDiameter(), abs_error<double>::value);
}

inline void RunTestAos() {
  TransactionalVector<CellTest<CompileTimeParam<Scalar>>> cells;
  RunTest(&cells);
}

inline void RunTestSoa() {
  auto cells = CellTest<CompileTimeParam<Soa>>::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace biology_module_op_test_internal
}  // namespace bdm

#endif  // UNIT_BIOLOGY_MODULE_OP_TEST_H_
