#ifndef UNIT_SIMULATION_OBJECT__VECTOR_TEST_H_
#define UNIT_SIMULATION_OBJECT__VECTOR_TEST_H_

#include "gtest/gtest.h"

#include "backend.h"
#include "gtest/gtest.h"
#include "simulation_object.h"
#include "simulation_object_util.h"
#include "simulation_object_vector.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

template <typename TBackend = Soa>
struct CompileTimeParam1;

BDM_SIM_CLASS_TEST(A, SimulationObject, CompileTimeParam1) {
  BDM_CLASS_HEADER(AExt, 1, id_);

 public:
  AExt() {}
  explicit AExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

BDM_SIM_CLASS_TEST(B, SimulationObject, CompileTimeParam1) {
  BDM_CLASS_HEADER(BExt, 1, id_);

 public:
  BExt() {}

  explicit BExt(int id) { id_[kIdx] = id; }

 private:
  vec<int> id_;
};

template <typename TBackend>
struct CompileTimeParam1 {
  template <typename TTBackend>
  using Self = CompileTimeParam1<TTBackend>;
  using Backend = TBackend;

  using SimulationBackend = Soa;
  using AtomicTypes = VariadicTypedef<A, B>;
};

inline void RunTest() {
  // setup resource manager
  using Rm = ResourceManager<CompileTimeParam1<Soa>>;
  auto rm = Rm::Get();

  ASSERT_EQ(2u, Rm::NumberOfTypes());

  auto as = rm->Get<A>();
  as->push_back(A(3));
  as->push_back(A(2));
  as->push_back(A(1));

  auto bs = rm->Get<B>();
  bs->push_back(B(8));
  bs->push_back(B(9));

  SimulationObjectVector<int, Rm> vector;
  EXPECT_EQ(0, vector[SoHandle(0, 0)]);
  EXPECT_EQ(0, vector[SoHandle(0, 1)]);
  EXPECT_EQ(0, vector[SoHandle(0, 2)]);
  EXPECT_EQ(0, vector[SoHandle(1, 0)]);
  EXPECT_EQ(0, vector[SoHandle(1, 1)]);

  vector[SoHandle(1, 0)] = 7;
  EXPECT_EQ(7, vector[SoHandle(1, 0)]);

  vector[SoHandle(0, 3)] = 5;
  EXPECT_EQ(5, vector[SoHandle(0, 3)]);
}

}  // namespace bdm
}  // namespace simulation_object_vector_test_internal

#endif  // UNIT_SIMULATION_OBJECT__VECTOR_TEST_H_
