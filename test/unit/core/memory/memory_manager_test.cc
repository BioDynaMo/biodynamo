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

#include "core/memory/memory_manager.h"
#include <gtest/gtest.h>
#include "core/sim_object/cell.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(ListTest, PushPopEmpty) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  EXPECT_TRUE(l.Empty());

  l.Push(&n1, &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n1);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n2);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n4);
  EXPECT_TRUE(l.Empty());

  EXPECT_EQ(l.Pop(), nullptr);
}

TEST(ListTest, PushTwice) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;
  Node n5;

  // one linked list: n1 -> n2 -> n3
  n1.next = &n2;
  n2.next = &n3;

  // second: n4 -> n5
  n4.next = &n5;

  EXPECT_TRUE(l.Empty());

  l.Push(&n1, &n3);
  l.Push(&n4, &n5);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n4);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n5);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n1);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n2);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n3);
  EXPECT_TRUE(l.Empty());

  EXPECT_EQ(l.Pop(), nullptr);
}

// Pop less elements than are in the list
TEST(ListTest, PopNThreadSafe_Less) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(3, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n3);

  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n4);
  EXPECT_TRUE(l.Empty());
}

// Pop more elements than are in the list
TEST(ListTest, PopNThreadSafe_More) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(8, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n4);

  EXPECT_TRUE(l.Empty());
}

// Pop exeaclty the number of elements that are in the list
TEST(ListTest, PopNThreadSafe_Equal) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(4, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n4);

  EXPECT_TRUE(l.Empty());
}

TEST(ListTest, PopNThreadSafe_Twice) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  {
    Node *head = nullptr;
    Node *tail = nullptr;
    l.PopNThreadSafe(2, &head, &tail);

    EXPECT_EQ(head, &n1);
    EXPECT_EQ(tail, &n2);
    EXPECT_EQ(tail->next, nullptr);
    EXPECT_FALSE(l.Empty());
  }

  {
    Node *head = nullptr;
    Node *tail = nullptr;
    l.PopNThreadSafe(2, &head, &tail);

    EXPECT_EQ(head, &n3);
    EXPECT_EQ(tail, &n4);
    EXPECT_EQ(tail->next, nullptr);
    EXPECT_TRUE(l.Empty());
  }
}
// -----------------------------------------------------------------------------
TEST(PageTreeTest, Ctor) {
  // 1 TB memory, 4k pages
  PageTree pt1(std::pow(1024, 4), 12);
  EXPECT_EQ(268435456, pt1.max_number_pages_);
  EXPECT_EQ(646, pt1.num_elements_per_node_);
  EXPECT_EQ(10u, pt1.bits_per_index_);
  EXPECT_EQ(0x3FF, pt1.idx0_mask_);
  EXPECT_EQ(0xFFC00, pt1.idx1_mask_);
  EXPECT_EQ(0x3FF00000, pt1.idx2_mask_);

  // 8 GB memory, 4k pages
  PageTree pt2(8 * std::pow(1024, 3), 12);
  EXPECT_EQ(2097152, pt2.max_number_pages_);
  EXPECT_EQ(128, pt2.num_elements_per_node_);
  EXPECT_EQ(7u, pt2.bits_per_index_);
  EXPECT_EQ(0x7F, pt2.idx0_mask_);
  EXPECT_EQ(0x3F80, pt2.idx1_mask_);
  EXPECT_EQ(0x1FC000, pt2.idx2_mask_);

  // 32 GB memory, 8k pages
  PageTree pt3(32 * std::pow(1024, 3), 13);
  EXPECT_EQ(4194304, pt3.max_number_pages_);
  EXPECT_EQ(162, pt3.num_elements_per_node_);
  EXPECT_EQ(8u, pt3.bits_per_index_);
  EXPECT_EQ(0xFF, pt3.idx0_mask_);
  EXPECT_EQ(0xFF00, pt3.idx1_mask_);
  EXPECT_EQ(0xFF0000, pt3.idx2_mask_);
}

TEST(PageTreeTest, AddOnePageAndGet) {
  PageTree pt(8 * std::pow(1024, 3), 12);
  auto* npa = reinterpret_cast<NumaPoolAllocator*>(0x456789);
  pt.AddPage(123123, npa);
  EXPECT_EQ(npa, pt.GetAllocator(123123));
}

// -----------------------------------------------------------------------------
TEST(MemoryManagerTest, New) {
  Simulation simulation(TEST_NAME);

  MemoryManager memory;
  // std::cout << memory.New(10) << std::endl;

  for (uint64_t i = 0; i < 10000; ++i) {
    auto so = new Cell();
    // auto* so = MemoryManager::New(sizeof(Cell));
    ASSERT_TRUE(so != nullptr);
    // std::cout << i << " " << so << std::endl;
  }
  // for (uint64_t i = 0; i < 100; ++i) {
  //   auto* so = MemoryManager::New(136);
  //   ASSERT_TRUE(so != nullptr);
  //   std::cout << i << " " << so << std::endl;
  // }
}

}  // namespace bdm
