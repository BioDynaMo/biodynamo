#include "gtest/gtest.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

// Constructors in SimulationObject are protected.
// Therefore create subclass
template <typename Backend>
struct ConcreteSimObject : public SimulationObject<SelectAllMembers, Backend> {
  FRIEND_TEST(SimulationObjectTest, is_fullSoaBackend);
};

TEST(SimulationObjectTest, is_fullScalarBackend) {
  ConcreteSimObject<ScalarBackend> object;
  EXPECT_TRUE(object.is_full());
}

// TODO(lukas) decide how vector backend should be called
// simd, vector ... in comparison to soa
TEST(SimulationObjectTest, is_fullVectorBackend) {
  ConcreteSimObject<VcVectorBackend> object;
  EXPECT_TRUE(object.is_full());
  object.SetSize(VcVectorBackend::kVecLen - 1);
  EXPECT_FALSE(object.is_full());
}

TEST(SimulationObjectTest, is_fullSoaBackend) {
  // SOA with one vector element
  ConcreteSimObject<VcSoaBackend> object;
  EXPECT_TRUE(object.is_full());
  object.clear();
  EXPECT_FALSE(object.is_full());
  object.size_last_vector_ = VcVectorBackend::kVecLen;
  EXPECT_TRUE(object.is_full());
}

TEST(SimulationObjectTest, sizeScalarBackend) {
  ConcreteSimObject<ScalarBackend> object;
  EXPECT_EQ(1u, object.size());
}

TEST(SimulationObjectTest, sizeVectorBackend) {
  ConcreteSimObject<VcVectorBackend> object;
  auto expected_size = VcVectorBackend::kVecLen;
  EXPECT_EQ(expected_size, object.size());
}

TEST(SimulationObjectTest, sizeSoaBackend) {
  ConcreteSimObject<VcSoaBackend> object;
  EXPECT_EQ(1u, object.size());
}

}  // namespace bdm
