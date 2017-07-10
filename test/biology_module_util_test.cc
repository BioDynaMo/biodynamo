#include "biology_module_util.h"
#include <vector>
#include "gtest/gtest.h"

namespace bdm {
namespace biology_module_util_test_internal {

using std::vector;

static bool gRunMethodCalled = false;
static bool gCopyCtorCalled = false;

/// Helper class to test run visitor
template <typename TSimulationObject>
struct RunTestBiologyModule {
  template <typename T>
  void Run(T* t) {
    EXPECT_EQ(expected_run_parameter_, t);
    gRunMethodCalled = true;
  }

  bool IsCopied(Event event) {
    EXPECT_TRUE(false) << "This method should not be called from RunVisitor";
    return false;
  }

  TSimulationObject* expected_run_parameter_ = nullptr;
};

/// Helper class to test copy visitor
struct CopyTestBiologyModule {
  CopyTestBiologyModule() {}

  CopyTestBiologyModule(const CopyTestBiologyModule& other) {
    gCopyCtorCalled = true;
    expected_event_ = other.expected_event_;
    is_copied_return_value_ = other.is_copied_return_value_;
  }

  template <typename T>
  void Run(T* t) {
    EXPECT_TRUE(false) << "This method should not be called from CopyVisitor";
  }

  bool IsCopied(Event event) const {
    EXPECT_EQ(expected_event_, event);
    return is_copied_return_value_;
  }

  Event expected_event_;
  bool is_copied_return_value_ = true;
};

struct TestSimulationObject {};

TEST(BiologyModuleUtilTest, RunVisitor) {
  TestSimulationObject sim_object;
  RunVisitor<TestSimulationObject> visitor(&sim_object);

  RunTestBiologyModule<TestSimulationObject> module;
  module.expected_run_parameter_ = &sim_object;
  variant<RunTestBiologyModule<TestSimulationObject>> variant = module;

  visit(visitor, variant);
  EXPECT_TRUE(gRunMethodCalled);
}

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
