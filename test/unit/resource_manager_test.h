#ifndef UNIT_RESOURCE_MANAGER_TEST_H_
#define UNIT_RESOURCE_MANAGER_TEST_H_

#include "resource_manager.h"

#include <vector>
#include "backend.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "type_util.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"
#include "variadic_template_parameter_util.h"
#include "simulation_object_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace resource_manager_test_internal {

BDM_SIM_OBJECT(A, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(AExt, 1, data_);

public:
  AExt() {}  // for ROOT I/O
  explicit AExt(int data) {
    data_[kIdx] = data;
  }

  int GetData() { return data_[kIdx]; }
  void SetData(int data) { data_[kIdx] = data; }

  vec<int> data_;
};

BDM_SIM_OBJECT(B, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(BExt, 1, data_);

public:
  BExt() {}  // for ROOT I/O
  explicit BExt(double data) {
    data_[kIdx] = data;
  }

  double GetData() { return data_[kIdx]; }
  void SetData(double data) { data_[kIdx] = data; }

  vec<double> data_;
};


template <typename TBackend, typename... Types>
struct CompileTimeParam {
  using SimulationBackend = TBackend;
  using AtomicTypes = VariadicTypedef<Types...>;
};

// FIXME this tests cause the following errors:
// runBiodynamoTestsMain_dict dictionary forward declarations' payload:78:202: error: no template named 'AExt'
// see rootcling_impl.cxx:L3668 GenerateFwdDeclString
template <typename Backend, typename TA, typename TB>
inline void RunIOTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();
  remove(ROOTFILE);

  // setup
  auto a_vector = rm->template Get<TA>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(A(12));
  a_vector->push_back(A(34));
  a_vector->push_back(A(42));

  auto b_vector = rm->template Get<TB>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(B(3.14));
  b_vector->push_back(B(6.28));

  DiffusionGrid* dgrid_1 = new DiffusionGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid(1, "Natrium", 0.2, 0.1, 1);
  rm->GetDiffusionGrids().push_back(dgrid_1);
  rm->GetDiffusionGrids().push_back(dgrid_2);

  // backup
  WritePersistentObject(ROOTFILE, "rm", *rm, "new");

  // restore
  ResourceManager<CTParam>* restored_rm = nullptr;
  GetPersistentObject(ROOTFILE, "rm", restored_rm);

  // validate
  ASSERT_EQ(3u, restored_rm->template Get<TA>()->size());
  EXPECT_EQ(12, (*restored_rm->template Get<TA>())[0].GetData());
  EXPECT_EQ(34, (*restored_rm->template Get<TA>())[1].GetData());
  EXPECT_EQ(42, (*restored_rm->template Get<TA>())[2].GetData());

  ASSERT_EQ(2u, rm->template Get<TB>()->size());
  EXPECT_NEAR(3.14, (*restored_rm->template Get<TB>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*restored_rm->template Get<TB>())[1].GetData(), kEpsilon);

  // FIXME segfaults
  // EXPECT_EQ(0, restored_rm->GetDiffusionGrids()[0]->GetSubstanceId());
  // EXPECT_EQ(1, restored_rm->GetDiffusionGrids()[1]->GetSubstanceId());
  // EXPECT_EQ("Kalium", restored_rm->GetDiffusionGrids()[0]->GetSubstanceName());
  // EXPECT_EQ("Natrium", restored_rm->GetDiffusionGrids()[1]->GetSubstanceName());
  // EXPECT_EQ(0.6,
  //           restored_rm->GetDiffusionGrids()[0]->GetDiffusionCoefficients()[0]);
  // EXPECT_EQ(0.8,
  //           restored_rm->GetDiffusionGrids()[1]->GetDiffusionCoefficients()[0]);

  remove(ROOTFILE);
}

inline void RunIOAosTest() {
  RunIOTest<Scalar, A, B>();
  RunIOTest<Scalar, SoaA, SoaB>();
}

inline void RunIOSoaTest() {
  RunIOTest<Soa, A, B>();
  RunIOTest<Soa, SoaA, SoaB>();
}

}  // namespace resource_manager_test_internal
}  // namespace bdm

#endif  // UNIT_RESOURCE_MANAGER_TEST_H_
