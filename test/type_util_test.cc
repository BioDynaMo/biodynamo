#include <typeinfo>
#include <string>
#include <gtest/gtest.h>
#include "type_util.h"

namespace bdm {

TEST(TypeUtilTest, True) {
  type_ternary_operator<true, int, double>::type data;
  EXPECT_EQ(std::string("i"), typeid(data).name());
}

TEST(TypeUtilTest, False) {
  type_ternary_operator<false, int, double>::type data;
  EXPECT_EQ(std::string("d"), typeid(data).name());
}

}  // namespace bdm
