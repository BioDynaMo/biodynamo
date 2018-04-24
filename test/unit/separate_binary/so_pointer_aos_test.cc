#include "unit/separate_binary/so_pointer_aos_test.h"

namespace bdm {
namespace so_pointer_aos_test_internal {

TEST_F(IOTest, SoPointerRmContainer_Aos) { IOTestSoPointerRmContainerAos(); }

}  // namespace so_pointer_aos_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
