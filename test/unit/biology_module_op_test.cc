#include "unit/biology_module_op_test.h"
#include "gtest/gtest.h"

namespace bdm {
namespace biology_module_op_test_internal {

TEST(BiologyModuleOpTest, ComputeAos) { RunTestAos(); }

TEST(BiologyModuleOpTest, ComputeSoa) { RunTestSoa(); }

}  // namespace biology_module_op_test_internal
}  // namespace bdm
