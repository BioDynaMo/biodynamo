#include "unit/variant_test.h"

namespace bdm {
namespace variant_test_internal {

// TEST(VariantTest, ConstructorAndVisit) {
//   Variant<int, float, char> vctor_i(3);
//   Variant<int, float, char> vctor_d(3.14);
//   Variant<int, float, char> vctor_c('b');

//   Variant<int, float, char> vassignment_op_i = 3;
//   Variant<int, float, char> vassignment_op_d = 3.14;
//   Variant<int, float, char> vassignment_op_c = 'b';

//   auto check_int = [](auto value) {
//     bool is_int = std::is_same<int, decltype(value)>::value;
//     EXPECT_TRUE(is_int);
//     EXPECT_EQ(3, value);
//   };
//   auto check_double = [](auto value) {
//     bool is_double = std::is_same<float, decltype(value)>::value;
//     EXPECT_TRUE(is_double);
//     EXPECT_EQ(3.14, value);
//   };
//   auto check_char = [](auto value) {
//     bool is_char = std::is_same<char, decltype(value)>::value;
//     EXPECT_TRUE(is_char);
//     EXPECT_EQ('b', value);
//   };

//   visit(check_int, vctor_i);
//   visit(check_int, vassignment_op_i);

//   visit(check_double, vctor_d);
//   visit(check_double, vassignment_op_d);

//   visit(check_char, vctor_c);
//   visit(check_char, vassignment_op_c);
// }

// TEST(VariantTest, GetIf) {
//   Variant<int, float, char> vint(3);

//   EXPECT_EQ(3, *(get_if<int>(&vint)));
//   EXPECT_EQ(nullptr, get_if<float>(&vint));
//   EXPECT_EQ(nullptr, get_if<char>(&vint));
// }

// TEST(VariantTest, IO) {
//   RunIOTestInt();
//   RunIOTestDouble();
//   RunIOTestChar();
// }

// TEST(VariantTest, IOVector) { RunIOVectorTest(); }

}  // namespace variant_test_internal
}  // namespace bdm
