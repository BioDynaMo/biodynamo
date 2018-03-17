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

TEST(is_tuple, All) {
  static_assert(!is_tuple<double>(), "double is NOT a std::tuple");
  static_assert(!is_tuple<std::vector<int>>(),
                "std::vector<int> is NOT std::tuple");

  static_assert(is_tuple<std::tuple<int>>(), "std::tuple<int> IS a std::tuple");
  static_assert(is_tuple<std::tuple<int, double>>(),
                "std::tuple<int, double> IS a std::tuple");
}

TEST(is_vector, All) {
  static_assert(!is_vector<double>(), "double is NOT a std::vector");
  static_assert(!is_vector<std::tuple<int, double>>(),
                "std::tuple<int> is NOT std::vector");

  static_assert(is_vector<std::vector<int>>(), "std::vector<int> IS a std::vector");
  static_assert(is_vector<std::vector<std::string>>(),
                "std::vector<std::string> IS a std::vector");
  static_assert(is_vector<std::vector<double, std::allocator<double> >>(),
                "std::vector<double, std::allocator<double> > IS a std::vector");
}

TEST(is_Variant, All) {
  static_assert(!is_Variant<double>(), "double is NOT a bdm::Variant");
  static_assert(!is_Variant<std::tuple<int, double>>(),
                "std::tuple<int, double> is NOT bdm::Variant");

  static_assert(is_Variant<bdm::Variant<int>>(), "bdm::Variant<int> IS a bdm::Variant");
  static_assert(is_Variant<bdm::Variant<int, double>>(),
                "bdm::Variant<int, double> IS a bdm::Variant");
}

}  // namespace bdm
