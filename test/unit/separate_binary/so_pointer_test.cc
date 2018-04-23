#include "unit/separate_binary/so_pointer_test.h"

namespace bdm {
namespace so_pointer_test_internal {

TEST(SoPointerTest, Aos) {
  TransactionalVector<SoPointerTestClass> sim_objects;
  SoPointerTestClass so(123);
  sim_objects.push_back(so);
  SoPointerTest<decltype(sim_objects), Scalar>(&sim_objects);
}

TEST(SoPointerTest, Soa) {
  auto sim_objects = SoPointerTestClass::NewEmptySoa();
  SoPointerTestClass so(123);
  sim_objects.push_back(so);
  SoPointerTest<decltype(sim_objects), Soa>(&sim_objects);
}

TEST_F(IOTest, SoPointerAnyContainer_Aos) { IOTestSoPointerAnyContainerAos(); }

TEST_F(IOTest, SoPointerAnyContainer_Soa) { IOTestSoPointerAnyContainerSoa(); }

TEST_F(IOTest, SoPointerRmContainer_Soa) { IOTestSoPointerRmContainerSoa(); }

TEST_F(IOTest, SoPointerNullptr_Aos) { IOTestSoPointerNullptrAos(); }

TEST_F(IOTest, SoPointerNullptr_Soa) { IOTestSoPointerNullptrSoa(); }

}  // namespace so_pointer_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
