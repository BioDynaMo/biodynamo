#ifndef UNIT_RESOURCE_MANAGER_TEST_H_
#define UNIT_RESOURCE_MANAGER_TEST_H_

#include "resource_manager.h"

#include <vector>
#include "backend.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "type_util.h"
#include "unit/test_util.h"
#include "variadic_template_parameter_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace resource_manager_test_internal {

using std::vector;

struct ASoa;
struct BSoa;

struct AScalar {
  using Backend = Scalar;
  template <typename Backend>
  using Self = typename type_ternary_operator<std::is_same<Soa, Backend>::value,
                                              ASoa, AScalar>::type;

  AScalar() {}  // for ROOT I/O
  explicit AScalar(int data) : data_(data) {}

  int GetData() { return data_; }

  int data_;

  ClassDefNV(AScalar, 1);
};

struct BScalar {
  using Backend = Scalar;
  template <typename Backend>
  using Self = typename type_ternary_operator<std::is_same<Soa, Backend>::value,
                                              BSoa, BScalar>::type;
  BScalar() {}  // for ROOT I/O
  explicit BScalar(double data) : data_(data) {}

  double GetData() { return data_; }

  double data_;

  ClassDefNV(BScalar, 1);
};

struct ASoa {
  using Backend = Soa;
  template <typename Backend>
  using Self = typename type_ternary_operator<std::is_same<Soa, Backend>::value,
                                              ASoa, AScalar>::type;

  vector<int> data_;
  size_t idx_ = 0;

  int GetData() { return data_[idx_]; }

  size_t size() const { return data_.size(); }  // NOLINT

  void clear() { data_.clear(); }  // NOLINT

  void push_back(const AScalar& a) { data_.push_back(a.data_); }  // NOLINT

  ASoa& operator[](size_t idx) {
    idx_ = idx;
    return *this;
  }

  ClassDefNV(ASoa, 1);
};

struct BSoa {
  using Backend = Soa;
  template <typename Backend>
  using Self = typename type_ternary_operator<std::is_same<Soa, Backend>::value,
                                              BSoa, BScalar>::type;

  vector<double> data_;
  size_t idx_ = 0;

  double GetData() { return data_[idx_]; }

  size_t size() const { return data_.size(); }  // NOLINT

  void clear() { data_.clear(); }  // NOLINT

  void push_back(const BScalar& b) { data_.push_back(b.data_); }  // NOLINT

  BSoa& operator[](size_t idx) {
    idx_ = idx;
    return *this;
  }

  ClassDefNV(BSoa, 1);
};

template <typename TBackend, typename... Types>
struct CompileTimeParam {
  using SimulationBackend = TBackend;
  using AtomicTypes = VariadicTypedef<Types...>;
};

template <typename Backend, typename A, typename B>
inline void RunIOTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();
  remove(ROOTFILE);

  // setup
  auto a_vector = rm->template Get<A>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(AScalar(12));
  a_vector->push_back(AScalar(34));
  a_vector->push_back(AScalar(42));

  auto b_vector = rm->template Get<B>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(BScalar(3.14));
  b_vector->push_back(BScalar(6.28));

  DiffusionGrid* dgrid_1 = new DiffusionGrid("Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid("Natrium", 0.2, 0.1, 1);
  rm->GetDiffusionGrids().push_back(dgrid_1);
  rm->GetDiffusionGrids().push_back(dgrid_2);

  // backup
  WritePersistentObject(ROOTFILE, "rm", *rm, "new");

  // restore
  ResourceManager<CTParam>* restored_rm = nullptr;
  GetPersistentObject(ROOTFILE, "rm", restored_rm);

  // validate
  ASSERT_EQ(3u, restored_rm->template Get<A>()->size());
  EXPECT_EQ(12, (*restored_rm->template Get<A>())[0].GetData());
  EXPECT_EQ(34, (*restored_rm->template Get<A>())[1].GetData());
  EXPECT_EQ(42, (*restored_rm->template Get<A>())[2].GetData());

  ASSERT_EQ(2u, rm->template Get<B>()->size());
  EXPECT_NEAR(3.14, (*restored_rm->template Get<B>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*restored_rm->template Get<B>())[1].GetData(), kEpsilon);

  EXPECT_EQ("Kalium", restored_rm->GetDiffusionGrids()[0]->GetSubstanceName());
  EXPECT_EQ("Natrium", restored_rm->GetDiffusionGrids()[1]->GetSubstanceName());
  EXPECT_EQ(0.6,
            restored_rm->GetDiffusionGrids()[0]->GetDiffusionCoefficients()[0]);
  EXPECT_EQ(0.8,
            restored_rm->GetDiffusionGrids()[1]->GetDiffusionCoefficients()[0]);

  remove(ROOTFILE);
}

inline void RunIOAosTest() {
  RunIOTest<Scalar, AScalar, BScalar>();
  RunIOTest<Scalar, ASoa, BSoa>();
}

inline void RunSoaTest() {
  RunIOTest<Soa, AScalar, BScalar>();
  RunIOTest<Soa, ASoa, BSoa>();
}

}  // namespace resource_manager_test_internal
}  // namespace bdm

#endif  // UNIT_RESOURCE_MANAGER_TEST_H_
