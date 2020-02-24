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

#ifndef CORE_MEMORY_MEMORY_MANAGER_H_
#define CORE_MEMORY_MEMORY_MANAGER_H_

#include <cassert>
#include <mutex>
#include <unordered_map>  // FIXME remove

#include "core/util/numa.h"
#include "core/util/spinlock.h"
#include "core/util/thread_info.h"

namespace bdm {

struct Node {
  Node* next = nullptr;
};

class List {
 public:
  List() {}
  List(const List& other) : head_(other.head_) {}

  Node* Pop();

  void PopNThreadSafe(uint64_t n, Node** head, Node** tail);

  // void Push(Node* node) {
  //   Node* tail = node;
  //   while (tail->next != nullptr) {
  //     tail = tail->next;
  //   }
  //   tail->next = head_->next;
  //   head_ = node;
  // }
  //
  // void PushThreadSafe(Node* node) {
  //   std::lock_guard<Spinlock> guard(lock_);
  //   Push(node);
  // }
  void Push(Node* head, Node* tail);

  void PushThreadSafe(Node* head, Node* tail);

  bool Empty() const;

 private:
  Node* head_ = nullptr;
  Spinlock lock_;
};

class NumaPoolAllocator {
 public:
  NumaPoolAllocator(uint64_t size, int nid);

  // NumaPoolAllocator(const NumaPoolAllocator* other) :
  //  total_size_(other->total_size_),
  //  size_(other->size_),
  //  nid_(other->nid_),
  //  memory_blocks_(other->nid_),

  ~NumaPoolAllocator();

  void* New(int tid);

  void Delete(void* p);

 private:
  const double kGrowthFactor = 2;
  uint64_t total_size_ = 0;
  uint64_t size_;
  int nid_;
  ThreadInfo* tinfo_;
  std::vector<std::pair<void*, std::size_t>> memory_blocks_;
  std::vector<List> free_lists_;  // one per thread
  List central_;
  std::mutex mutex_; // TODO replace with Spinlock

  void AllocNewMemoryBlock(std::size_t size);

  void CreateFreeList(void* block, uint64_t mem_block_size, Node** head,
                      Node** tail);

  void ProcessPages(void* block, uint64_t mem_block_size);
};

struct TreeNode;

union TreeData {
  TreeNode* node_ = nullptr;
  NumaPoolAllocator* data_;
};

struct TreeNode {
  TreeNode(uint64_t num_elements);
  std::vector<TreeData> nodes_;
  Spinlock lock_;
};

class PageTree {
public:
  PageTree(uint64_t total_memory, uint64_t page_shift);

  NumaPoolAllocator* GetAllocator(uint64_t page_number) const;

  void AddPage(uint64_t page, NumaPoolAllocator* npa);

private:
  uint64_t max_number_pages_;
  uint64_t num_elements_per_node_;
  uint64_t bits_per_index_;
  uint64_t idx0_mask_;
  uint64_t idx1_mask_;
  uint64_t idx2_mask_;
  TreeNode* head_;

  std::array<uint64_t, 3> GetIndices(uint64_t page_number) const;

  friend class PageTreeTest_Ctor_Test;
};

// FIXME move to separate source file
class PoolAllocator {
 public:
  PoolAllocator(std::size_t size);

  PoolAllocator(const PoolAllocator& other);

  ~PoolAllocator();

  void* New(std::size_t size);

  void Delete(void* p);

 private:
  std::size_t size_;
  ThreadInfo* tinfo_;
  std::vector<NumaPoolAllocator*> numa_allocators_;
};

class MemoryManager {
 public:
   static constexpr uint64_t kPageSize = 4096;
   static constexpr uint64_t kPageShift = 12;

  static PageTree page_tree_;

  static void* New(std::size_t size);

  static void Delete(void* p);

 private:
  static std::unordered_map<std::size_t, PoolAllocator> allocators_;
};

}  // namespace bdm

#endif  // CORE_MEMORY_MEMORY_MANAGER_H_
