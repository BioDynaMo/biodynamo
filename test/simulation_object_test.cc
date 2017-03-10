#include "gtest/gtest.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

namespace bdm {

// Constructors in BdmSimObject are protected.
// Therefore create subclass
template <typename Backend>
struct ConcreteSimObject : public BdmSimObject<SelectAllMembers, Backend> {
  FRIEND_TEST(BdmSimObjectTest, is_fullSoaBackend);
};

TEST(BdmSimObjectTest, is_fullScalarBackend) {
  ConcreteSimObject<ScalarBackend> object;
  EXPECT_TRUE(object.is_full());
}

// TODO(lukas) decide how vector backend should be called
// simd, vector ... in comparison to soa
TEST(BdmSimObjectTest, is_fullVectorBackend) {
  ConcreteSimObject<VcBackend> object;
  EXPECT_TRUE(object.is_full());
  object.SetSize(VcBackend::kVecLen - 1);
  EXPECT_FALSE(object.is_full());
}

TEST(BdmSimObjectTest, is_fullSoaBackend) {
  // SOA with one vector element
  ConcreteSimObject<VcSoaBackend> object;
  EXPECT_TRUE(object.is_full());
  object.clear();
  EXPECT_FALSE(object.is_full());
  object.size_last_vector_ = VcBackend::kVecLen;
  EXPECT_TRUE(object.is_full());
}

TEST(BdmSimObjectTest, sizeScalarBackend) {
  ConcreteSimObject<ScalarBackend> object;
  EXPECT_EQ(1u, object.size());
}

TEST(BdmSimObjectTest, sizeVectorBackend) {
  ConcreteSimObject<VcBackend> object;
  auto expected_size = VcBackend::kVecLen;
  EXPECT_EQ(expected_size, object.size());
}

TEST(BdmSimObjectTest, sizeSoaBackend) {
  ConcreteSimObject<VcSoaBackend> object;
  EXPECT_EQ(1u, object.size());
}

}  // namespace bdm
