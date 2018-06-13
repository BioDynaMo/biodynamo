#ifndef UNIT_VARIANT_TEST_H_
#define UNIT_VARIANT_TEST_H_

#include <vector>
#include "gtest/gtest.h"
#include "io_util.h"
#include "unit/test_util.h"
#include "variant.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace variant_test_internal {

// inline void RunIOTestInt() {
//   // write
//   Variant<int, float, char> v = 12;
//   WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

//   // read
//   Variant<int, float, char>* v_r = nullptr;
//   GetPersistentObject(ROOTFILE, "variant", v_r);

//   // check
//   visit([](auto value) { EXPECT_EQ(12, value); }, *v_r);

//   remove(ROOTFILE);
// }

// inline void RunIOTestDouble() {
//   // write
//   Variant<int, float, char> v = 3.14;
//   WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

//   // read
//   Variant<int, float, char>* v_r = nullptr;
//   GetPersistentObject(ROOTFILE, "variant", v_r);

//   // check
//   visit([](auto value) { EXPECT_NEAR(3.14, value, abs_error<float>::value); },
//         *v_r);

//   remove(ROOTFILE);
// }

// inline void RunIOTestChar() {
//   // write
//   Variant<int, float, char> v = 'c';
//   WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

//   // read
//   Variant<int, float, char>* v_r = nullptr;
//   GetPersistentObject(ROOTFILE, "variant", v_r);

//   // check
//   visit([](auto value) { EXPECT_EQ('c', value); }, *v_r);

//   remove(ROOTFILE);
// }

// inline void RunIOVectorTest() {
//   // write
//   Variant<int, float, char> v = 3.14;
//   std::vector<decltype(v)> variant_v;
//   variant_v.push_back(v);
//   WritePersistentObject(ROOTFILE, "variant_v", variant_v, "RECREATE");

//   // read
//   decltype(variant_v)* variant_v_r = nullptr;
//   GetPersistentObject(ROOTFILE, "variant_v", variant_v_r);

//   // check
//   visit([](auto value) { EXPECT_NEAR(3.14, value, abs_error<float>::value); },
//         (*variant_v_r)[0]);

//   remove(ROOTFILE);
// }

}  // namespace variant_test_internal
}  // namespace bdm

#endif  //  UNIT_VARIANT_TEST_H_
