#include "core/container/shared_data.h"
#include <gtest/gtest.h>
#include <vector>

namespace bdm {

// Test if resize and size method work correctly.
TEST(SharedDataTest, ReSize) {
  SharedData<int> sdata(10);
  EXPECT_EQ(sdata.size(), 10);
  sdata.resize(20);
  EXPECT_EQ(sdata.size(), 20);
}

// Test if underlying data type has changed, e.g. if it is a standard vector.
// A std::vector is usually (ond 64bit systems) 24 bytes, independet of the
// data type stored in the vector.
TEST(SharedDataTest, DataType) {
  EXPECT_EQ(sizeof(typename SharedData<char>::Data),
            sizeof(typename std::vector<char>));
}

// Test if shared data is occupying full cache lines.
TEST(SharedDataTest, CacheLineAlignment) {
  // Test alignment of int
  EXPECT_EQ(
      std::alignment_of<typename SharedData<int>::Data::value_type>::value, 64);
  // Test alignment of float
  EXPECT_EQ(
      std::alignment_of<typename SharedData<float>::Data::value_type>::value,
      64);
  // Test alignment of double
  EXPECT_EQ(
      std::alignment_of<typename SharedData<double>::Data::value_type>::value,
      64);
  // Test alignment of double[8], e.g. max cache line capacity
  EXPECT_EQ(std::alignment_of<
                typename SharedData<double[8]>::Data::value_type>::value,
            64);
  // Test alignment of 70 byte struct
  EXPECT_EQ(
      std::alignment_of<typename SharedData<char[70]>::Data::value_type>::value,
      64);
  // Test alignment of 130 byte struct
  EXPECT_EQ(std::alignment_of<
                typename SharedData<char[130]>::Data::value_type>::value,
            64);
  // Test size of vector components int
  EXPECT_EQ(sizeof(typename SharedData<int>::Data::value_type), 64);
  // Test size of vector components float
  EXPECT_EQ(sizeof(typename SharedData<float>::Data::value_type), 64);
  // Test size of vector components double
  EXPECT_EQ(sizeof(typename SharedData<double>::Data::value_type), 64);
  // Test size of vector components double, e.g. max cache line capacity
  EXPECT_EQ(sizeof(typename SharedData<double[8]>::Data::value_type), 64);
  // Test size of vector components for 70 byte strct
  EXPECT_EQ(sizeof(typename SharedData<char[70]>::Data::value_type), 128);
  // Test size of vector components for 130 byte struct
  EXPECT_EQ(sizeof(typename SharedData<char[130]>::Data::value_type), 192);
}

}  // namespace bdm