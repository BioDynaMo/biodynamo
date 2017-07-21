#include "biology_module_util_test.h"

namespace bdm {
namespace biology_module_util_test_internal {

TEST(BiologyModuleUtilTest, RunVisitor) { RunRunVisitor(); }

TEST(BiologyModuleUtilTest, CopyVisitorIsCopied) {
  vector<variant<CopyTestBiologyModule>> destination_module_vector;
  CopyVisitor<vector<variant<CopyTestBiologyModule>>> visitor(
      Event::kCellDivision, &destination_module_vector);

  CopyTestBiologyModule module;
  module.expected_event_ = Event::kCellDivision;
  variant<CopyTestBiologyModule> variant = module;

  gCopyCtorCalled = false;
  visit(visitor, variant);
  EXPECT_EQ(1u, destination_module_vector.size());
  EXPECT_TRUE(gCopyCtorCalled);
}

TEST(BiologyModuleUtilTest, CopyVisitorIsNotCopied) {
  vector<variant<CopyTestBiologyModule>> destination_module_vector;
  CopyVisitor<vector<variant<CopyTestBiologyModule>>> visitor(
      Event::kCellDivision, &destination_module_vector);

  CopyTestBiologyModule module;
  module.expected_event_ = Event::kCellDivision;
  module.is_copied_return_value_ = false;
  variant<CopyTestBiologyModule> variant = module;

  gCopyCtorCalled = false;
  visit(visitor, variant);
  EXPECT_EQ(0u, destination_module_vector.size());
  EXPECT_FALSE(gCopyCtorCalled);
}

}  // namespace biology_module_util_test_internal
}  // namespace bdm
