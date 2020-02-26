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
namespace memory_manager_detail {

TEST(ListTest, PushFrontPopFront) {
  List l(4);

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  EXPECT_TRUE(l.Empty());

  l.PushFront(&n1);
  l.PushFront(&n2);
  l.PushFront(&n3);
  l.PushFront(&n4);

  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n4);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n2);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n1);
  EXPECT_TRUE(l.Empty());

  EXPECT_EQ(l.PopFront(), nullptr);
}

TEST(ListTest, PushBackN) {
  List l(4);

  Node n1;
  Node n2;
  Node n3;
  Node n4;
  Node n5;

  // one linked list: n1 -> n2 -> n3
  n2.next = &n3;
  n3.next = &n4;
  n4.next = &n5;

  EXPECT_TRUE(l.Empty());

  l.PushFront(&n1);
  EXPECT_FALSE(l.Empty());

  l.PushBackN(&n2, &n5);

  EXPECT_EQ(l.PopFront(), &n1);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n2);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n4);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n5);
  EXPECT_TRUE(l.Empty());
}

TEST(ListTest, PopBackN) {
  List l(4);

  Node n1;
  Node n2;
  Node n3;
  Node n4;
  Node n5;

  // one linked list: n1 -> n2 -> n3
  n2.next = &n3;
  n3.next = &n4;
  n4.next = &n5;

  EXPECT_TRUE(l.Empty());

  l.PushFront(&n1);
  EXPECT_FALSE(l.Empty());

  l.PushBackN(&n2, &n5);

  Node* head = nullptr;
  Node* tail = nullptr;
  l.PopBackN(&head, &tail);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n1);
  EXPECT_TRUE(l.Empty());
}

TEST(ListTest, PushBackNPopBackN) {
  List l(4);

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  // one linked list: n1 -> n2 -> n3
  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  EXPECT_TRUE(l.Empty());

  l.PushBackN(&n1, &n4);

  Node* head = nullptr;
  Node* tail = nullptr;
  l.PopBackN(&head, &tail);
  // cannot get elements back if only N elements are stored inside
  EXPECT_EQ(nullptr, head);
  EXPECT_EQ(nullptr, tail);
  EXPECT_FALSE(l.Empty());

  Node n5;
  l.PushFront(&n5);
  l.PopBackN(&head, &tail);
  EXPECT_EQ(&n1, head);
  EXPECT_EQ(&n4, tail);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.PopFront(), &n5);
  EXPECT_TRUE(l.Empty());
}

TEST(ListTest, CreateRemoveSkipListEntry) {
  List l(2);

  Node n1;
  Node n2;
  Node n3;
  Node n4;
  Node n5;
  Node n6;
  Node* head = nullptr;
  Node* tail = nullptr;

  l.PushFront(&n1);
  l.PushFront(&n2);
  l.PushFront(&n3); // skip list item should be created
  l.PopFront();     // skip list item should be removed

  l.PopBackN(&head, &tail);
  EXPECT_EQ(nullptr, head);
  EXPECT_EQ(nullptr, tail);

  l.PushFront(&n4);  // skip list item should be created again
  l.PushFront(&n5);

  l.PopBackN(&head, &tail);
  EXPECT_EQ(&n2, head);
  EXPECT_EQ(&n1, tail);
  EXPECT_EQ(2u, l.Size());
}

TEST(ListTest, PushFrontPushBackN_LargeScale) {
  List l(4);

  for (uint64_t i = 0; i < 402; i++) {
    l.PushFrontThreadSafe(new Node());
  }
  EXPECT_EQ(402u, l.Size());

  Node* head = nullptr;
  Node* tail = nullptr;
  for (uint64_t i = 0; i < 100; i++) {
    l.PopBackNThreadSafe(&head, &tail);
    EXPECT_TRUE(head != nullptr);
    EXPECT_TRUE(tail != nullptr);
  }
  EXPECT_EQ(2u, l.Size());

}

// -----------------------------------------------------------------------------
TEST(AllocatedBlock, PerfectAligned) {
  uint64_t size_n_pages = 65536;
  auto* end = reinterpret_cast<char*>(2*size_n_pages);
  AllocatedBlock block = {0, end, 0};
  EXPECT_FALSE(block.IsFullyInitialized());

  char* batch = nullptr;
  uint64_t size = 0;
  block.GetNextPageBatch(size_n_pages, &batch, &size);
  EXPECT_EQ(0, batch);
  EXPECT_EQ(size_n_pages, size);

  EXPECT_FALSE(block.IsFullyInitialized());

  block.GetNextPageBatch(size_n_pages, &batch, &size);
  EXPECT_EQ(reinterpret_cast<char*>(size_n_pages), batch);
  EXPECT_EQ(size_n_pages, size);

  EXPECT_TRUE(block.IsFullyInitialized());
}

TEST(AllocatedBlock, NotPerfectlyAligned) {
  uint64_t size_n_pages = 65536;
  auto* start = reinterpret_cast<char*>(4096);
  auto* end = reinterpret_cast<char*>(2*size_n_pages+4096);
  auto* initialized = reinterpret_cast<char*>(size_n_pages);
  AllocatedBlock block = {start, end, initialized};
  EXPECT_FALSE(block.IsFullyInitialized());

  char* batch = nullptr;
  uint64_t size = 0;
  block.GetNextPageBatch(size_n_pages, &batch, &size);
  EXPECT_EQ(initialized, batch);
  EXPECT_EQ(size_n_pages, size);

  EXPECT_FALSE(block.IsFullyInitialized());

  block.GetNextPageBatch(size_n_pages, &batch, &size);
  EXPECT_EQ(reinterpret_cast<char*>(2*size_n_pages), batch);
  EXPECT_EQ(4096u, size);

  EXPECT_TRUE(block.IsFullyInitialized());
}

// -----------------------------------------------------------------------------
TEST(NumaPoolAllocatorTest, RoundUpTo) {
  EXPECT_EQ(0u, NumaPoolAllocator::RoundUpTo(0, 4096));
  EXPECT_EQ(4096u, NumaPoolAllocator::RoundUpTo(1, 4096));
  EXPECT_EQ(4096u, NumaPoolAllocator::RoundUpTo(4096, 4096));
  EXPECT_EQ(8192u, NumaPoolAllocator::RoundUpTo(4097, 4096));
}

// -----------------------------------------------------------------------------
TEST(MemoryManagerTest, New) {
  Simulation simulation(TEST_NAME);

  uint64_t page_shift = static_cast<uint64_t>(std::log2(sysconf(_SC_PAGESIZE)));
  uint64_t aligned_pages_shift_ = 8;

  for (uint64_t i = 0; i < 1000; ++i) {
    auto* so = new Cell();
    ASSERT_TRUE(so != nullptr);

    // check if we can find the numa pool allocator pointer at the beginning of
    // the N aligned pages that is used to free the memory once `so` is deleted
    auto addr = reinterpret_cast<uint64_t>(so);
    auto page_number = addr >> (page_shift + aligned_pages_shift_);
    auto* page_addr = reinterpret_cast<char*>(page_number << (page_shift + aligned_pages_shift_));

    auto* npa = *reinterpret_cast<NumaPoolAllocator**>(page_addr);

    EXPECT_EQ(sizeof(Cell), npa->GetSize());
    delete so;
  }
}

}  // namespace memory_manager_detail
}  // namespace bdm
