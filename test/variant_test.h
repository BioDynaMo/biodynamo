#ifndef VARIANT_TEST_H_
#define VARIANT_TEST_H_

#include "gtest/gtest.h"
#include "variant.h"
#include "io_util.h"
#include "test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace variant_test_internal {

inline void RunIOTestInt() {
  // write
  Variant<int, double, char> v = 12;
  WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

  // read
  Variant<int, double, char>* v_r = nullptr;
  GetPersistentObject(ROOTFILE, "variant", v_r);

  // check
  visit([](auto value) { EXPECT_EQ(12, value); }, *v_r);

  remove(ROOTFILE);
}

inline void RunIOTestDouble() {
  // write
  Variant<int, double, char> v = 3.14;
  WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

  // read
  Variant<int, double, char>* v_r = nullptr;
  GetPersistentObject(ROOTFILE, "variant", v_r);

  // check
  visit([](auto value) { EXPECT_NEAR(3.14, value, abs_error<double>::value); },
        *v_r);

  remove(ROOTFILE);
}

inline void RunIOTestChar() {
  // write
  Variant<int, double, char> v = 'c';
  WritePersistentObject(ROOTFILE, "variant", v, "RECREATE");

  // read
  Variant<int, double, char>* v_r = nullptr;
  GetPersistentObject(ROOTFILE, "variant", v_r);

  // check
  visit([](auto value) { EXPECT_EQ('c', value); }, *v_r);

  remove(ROOTFILE);
}

inline void RunIOVectorTest() {
  // write
  Variant<int, double, char> v = 3.14;
  std::vector<decltype(v)> variant_v;
  variant_v.push_back(v);
  WritePersistentObject(ROOTFILE, "variant_v", variant_v, "RECREATE");

  // read
  decltype(variant_v)* variant_v_r = nullptr;
  GetPersistentObject(ROOTFILE, "variant_v", variant_v_r);

  // check
  visit([](auto value) { EXPECT_NEAR(3.14, value, abs_error<double>::value); },
        (*variant_v_r)[0]);

  remove(ROOTFILE);
}

}  // namespace variant_test_internal
}  // namespace bdm

#endif  //  VARIANT_TEST_H_
