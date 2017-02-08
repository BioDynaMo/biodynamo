#include <gtest/gtest.h>
#include "test_util.h"

namespace bdm {

TEST(TestUtilTest, abs_errorTypeTrait) {
  EXPECT_DOUBLE_EQ(abs_error<float>::value, 1e-6);
  EXPECT_DOUBLE_EQ(abs_error<double>::value, 1e-9);
  // abs_error<int>::value // must not compile -> todo compile error test suite
}

}  // namespace bdm
