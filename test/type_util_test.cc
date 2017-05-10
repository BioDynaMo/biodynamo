#include "type_util.h"
#include <string>
#include <typeinfo>
#include "cell.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(TypeTernaryOperatorTest, True) {
  type_ternary_operator<true, int, double>::type data;
  EXPECT_EQ(std::string("i"), typeid(data).name());
}

TEST(TypeTernaryOperatorTest, False) {
  type_ternary_operator<false, int, double>::type data;
  EXPECT_EQ(std::string("d"), typeid(data).name());
}

TEST(is_soaTest, All) {
  EXPECT_FALSE(is_soa<Scalar>::value);
  EXPECT_TRUE(is_soa<Soa>::value);
  EXPECT_TRUE(is_soa<SoaRef>::value);
}

}  // namespace bdm
