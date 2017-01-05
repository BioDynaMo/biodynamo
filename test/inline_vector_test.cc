#include <new>

#include "gtest/gtest.h"
#include "inline_vector.h"

size_t operator_new_calls_ = 0;

/// overload new[] operator to test how often heap memory is allocated
void* operator new[](std::size_t sz) {
  void* p = malloc(sz);
  operator_new_calls_++;
  return p;
}

/// required for valgrind tests
/// otherwise "Mismatched free() / delete / delete []" errors
void operator delete[](void* ptr) { free(ptr); }

namespace bdm {

TEST(InlineVectorTest, NoHeapAllocations) {
  size_t new_calls_before = operator_new_calls_;
  InlineVector<int, 3> vector;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);

  EXPECT_EQ(0u, operator_new_calls_ - new_calls_before);
  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(1, vector[1]);
  EXPECT_EQ(2, vector[2]);
}

TEST(InlineVectorTest, OneHeapAllocation) {
  size_t new_calls_before = operator_new_calls_;
  InlineVector<int, 3> vector;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);

  EXPECT_EQ(1u, operator_new_calls_ - new_calls_before);
  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(1, vector[1]);
  EXPECT_EQ(2, vector[2]);
  EXPECT_EQ(3, vector[3]);
}

TEST(InlineVectorTest, TwoHeapAllocations) {
  size_t new_calls_before = operator_new_calls_;
  InlineVector<int, 3> vector;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);

  EXPECT_EQ(2u, operator_new_calls_ - new_calls_before);
  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(1, vector[1]);
  EXPECT_EQ(2, vector[2]);
  EXPECT_EQ(3, vector[3]);
  EXPECT_EQ(4, vector[4]);
}

TEST(InlineVectorTest, size) {
  InlineVector<int, 3> vector;
  EXPECT_EQ(0u, vector.size());
  vector.push_back(0);
  EXPECT_EQ(1u, vector.size());
  vector.push_back(1);
  EXPECT_EQ(2u, vector.size());
  vector.push_back(2);
  EXPECT_EQ(3u, vector.size());
  vector.push_back(3);
  EXPECT_EQ(4u, vector.size());
  vector.push_back(4);
  EXPECT_EQ(5u, vector.size());
}

TEST(InlineVectorTest, EqualsOperatorNoHeap) {
  InlineVector<int, 2> lhs;
  lhs.push_back(0);
  lhs.push_back(1);

  InlineVector<int, 2> rhs;
  rhs.push_back(0);
  rhs.push_back(1);

  EXPECT_TRUE(lhs == rhs);

  rhs.push_back(3);
  EXPECT_FALSE(lhs == rhs);

  lhs.push_back(4);
  EXPECT_FALSE(lhs == rhs);
}

TEST(InlineVectorTest, EqualsOperatorWithHeap) {
  InlineVector<int, 2> lhs;
  lhs.push_back(0);
  lhs.push_back(1);
  lhs.push_back(2);
  lhs.push_back(3);

  InlineVector<int, 2> rhs;
  rhs.push_back(0);
  rhs.push_back(1);
  rhs.push_back(2);
  rhs.push_back(3);

  EXPECT_TRUE(lhs == rhs);

  rhs.push_back(5);
  EXPECT_FALSE(lhs == rhs);

  lhs.push_back(7);
  EXPECT_FALSE(lhs == rhs);
}

}  // namespace bdm
