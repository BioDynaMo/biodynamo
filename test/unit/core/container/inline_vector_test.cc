// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "unit/core/container/inline_vector_test.h"
#include <new>

size_t operator_new_calls_ = 0;

/// overload new[] operator to test how often heap memory is allocated
void* operator new[](std::size_t sz) {
  void* p = malloc(sz);
  operator_new_calls_++;
  return p;
}

/// required for valgrind tests
/// otherwise "Mismatched free() / delete / delete []" errors
void operator delete[](void* ptr) noexcept { free(ptr); }

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

TEST(InlineVectorTest, reserve) {
  InlineVector<int, 3> vector;
  vector.reserve(5);
  size_t new_calls_before = operator_new_calls_;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);

  EXPECT_EQ(0u, operator_new_calls_ - new_calls_before);
  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(1, vector[1]);
  EXPECT_EQ(2, vector[2]);
  EXPECT_EQ(3, vector[3]);
  EXPECT_EQ(4, vector[4]);
}

TEST(InlineVectorTest, reserveRobustnessTest) {
  // new capacity smaller than current
  size_t new_calls_before = operator_new_calls_;
  InlineVector<int, 3> vector;
  vector.reserve(2);

  EXPECT_EQ(0u, operator_new_calls_ - new_calls_before);
}

TEST(InlineVectorTest, capacity) {
  InlineVector<int, 3> vector;
  EXPECT_EQ(3u, vector.capacity());
  vector.push_back(0);
  EXPECT_EQ(3u, vector.capacity());
  vector.push_back(1);
  EXPECT_EQ(3u, vector.capacity());
  vector.push_back(2);
  EXPECT_EQ(3u, vector.capacity());
  vector.push_back(3);
  EXPECT_EQ(4u, vector.capacity());
  vector.push_back(4);
  EXPECT_EQ(6u, vector.capacity());
}

TEST(InlineVectorTest, clear) {
  InlineVector<int, 3> vector;
  for (size_t i = 0; i < 8; i++) {
    vector.push_back(i);
  }
  size_t capacity_before_clear = vector.capacity();
  vector.clear();
  EXPECT_EQ(capacity_before_clear, vector.capacity());
  EXPECT_EQ(0u, vector.size());
}

TEST(InlineVectorTest, CopyCtor) {
  InlineVector<int, 3> vector;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);
  InlineVector<int, 3> vector_cpy(vector);

  EXPECT_EQ(vector.size(), vector_cpy.size());
  EXPECT_EQ(vector.capacity(), vector_cpy.capacity());
  EXPECT_EQ(0, vector_cpy[0]);
  EXPECT_EQ(1, vector_cpy[1]);
  EXPECT_EQ(2, vector_cpy[2]);
  EXPECT_EQ(3, vector_cpy[3]);
  EXPECT_EQ(4, vector_cpy[4]);
}

TEST(InlineVectorTest, MoveCtor) {
  InlineVector<int, 3> vector;
  vector.push_back(0);
  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);

  size_t new_calls_before = operator_new_calls_;
  InlineVector<int, 3> moved_vector(std::move(vector));

  EXPECT_EQ(0u, operator_new_calls_ - new_calls_before);
  EXPECT_EQ(5u, moved_vector.size());
  EXPECT_EQ(6u, moved_vector.capacity());
  EXPECT_EQ(0, moved_vector[0]);
  EXPECT_EQ(1, moved_vector[1]);
  EXPECT_EQ(2, moved_vector[2]);
  EXPECT_EQ(3, moved_vector[3]);
  EXPECT_EQ(4, moved_vector[4]);
}

TEST(InlineVectorTest, AssignmentOperator) {
  InlineVector<int, 3> vector_cpy;
  {
    InlineVector<int, 3> vector;
    vector.push_back(0);
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    vector_cpy = vector;
  }
  EXPECT_EQ(5u, vector_cpy.size());
  EXPECT_EQ(6u, vector_cpy.capacity());
  EXPECT_EQ(0, vector_cpy[0]);
  EXPECT_EQ(1, vector_cpy[1]);
  EXPECT_EQ(2, vector_cpy[2]);
  EXPECT_EQ(3, vector_cpy[3]);
  EXPECT_EQ(4, vector_cpy[4]);
}

TEST(InlineVectorTest, MoveAssignmentOperator) {
  InlineVector<int, 3> moved_vector;
  {
    InlineVector<int, 3> vector;
    vector.push_back(0);
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);

    moved_vector = std::move(vector);
  }
  EXPECT_EQ(5u, moved_vector.size());
  EXPECT_EQ(6u, moved_vector.capacity());
  EXPECT_EQ(0, moved_vector[0]);
  EXPECT_EQ(1, moved_vector[1]);
  EXPECT_EQ(2, moved_vector[2]);
  EXPECT_EQ(3, moved_vector[3]);
  EXPECT_EQ(4, moved_vector[4]);
}

TEST(InlineVectorTest, EqualsOperatorNoHeap) {
  InlineVector<int, 3> lhs;
  lhs.push_back(0);
  lhs.push_back(1);

  InlineVector<int, 3> rhs;
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

TEST(InlineVectorTest, ForEachNoHeap) {
  InlineVector<int, 4> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);

  std::vector<int> expected = {0, 1, 2};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(3u, cnt);
}

TEST(InlineVectorTest, ForEachWithHeap) {
  InlineVector<int, 2> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);
  iv.push_back(4);

  std::vector<int> expected = {0, 1, 2, 3, 4};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(5u, cnt);
}

TEST(InlineVectorTest, Iterator) {
  InlineVector<int, 2> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);
  iv.push_back(4);

  auto it = iv.begin();
  EXPECT_TRUE(it == iv.begin());
  it++;
  EXPECT_TRUE(it != iv.begin());
  EXPECT_EQ(1, *it);
  ++it;
  EXPECT_EQ(2, *it);
  it += 2;
  EXPECT_EQ(4, *it);
  it -= 2;
  EXPECT_EQ(2, *it);
  --it;
  EXPECT_EQ(1, *it);
  it--;
  EXPECT_EQ(0, *it);

  it = iv.begin() + 3;
  EXPECT_EQ(3, *it);
  it = iv.end() - 3;
  EXPECT_EQ(2, *it);
  auto it1 = iv.begin() + 1;
  it = it1 + it1;
  EXPECT_EQ(2, *it);
  it = it1 + it1 - it1;
  EXPECT_EQ(1, *it);
  it += it1;
  EXPECT_EQ(2, *it);
  it -= it1;
  EXPECT_EQ(1, *it);
}

TEST(InlineVectorTest, erase1) {
  InlineVector<int, 3> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);
  iv.push_back(4);

  auto it = iv.erase(iv.begin() + 1);
  EXPECT_EQ(2, *it);

  std::vector<int> expected = {0, 2, 3, 4};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(4u, cnt);
  EXPECT_EQ(4u, iv.size());
}

TEST(InlineVectorTest, erase2) {
  InlineVector<int, 3> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);
  iv.push_back(4);

  auto it = iv.erase(iv.begin() + 3);
  EXPECT_EQ(4, *it);

  std::vector<int> expected = {0, 1, 2, 4};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(4u, cnt);
  EXPECT_EQ(4u, iv.size());
}

TEST(InlineVectorTest, erase_last) {
  InlineVector<int, 3> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);
  iv.push_back(4);

  auto it = iv.erase(iv.begin() + 4);
  EXPECT_EQ(iv.end(), it);

  std::vector<int> expected = {0, 1, 2, 3};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(4u, cnt);
  EXPECT_EQ(4u, iv.size());
}

TEST(InlineVectorTest, erase_no_heap) {
  InlineVector<int, 3> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);

  auto it = iv.erase(iv.begin() + 1);
  EXPECT_EQ(2, *it);

  std::vector<int> expected = {0, 2};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(2u, cnt);
  EXPECT_EQ(2u, iv.size());
}

TEST(InlineVectorTest, erase_no_heap_last) {
  InlineVector<int, 3> iv;
  iv.push_back(0);
  iv.push_back(1);
  iv.push_back(2);

  auto it = iv.erase(iv.begin() + 2);
  EXPECT_EQ(iv.end(), it);

  std::vector<int> expected = {0, 1};
  uint16_t cnt = 0;
  for (auto el : iv) {
    EXPECT_EQ(expected[cnt++], el);
  }
  EXPECT_EQ(2u, cnt);
  EXPECT_EQ(2u, iv.size());
}

#ifdef USE_DICT
TEST(InlineVectorTest, IO) { inline_vector_test_internal::RunIOTest(); }
#endif  // USE_DICT

}  // namespace bdm
