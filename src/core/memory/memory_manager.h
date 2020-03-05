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
#include <list>
#include <vector>

#include "core/container/flatmap.h"
#include "core/util/numa.h"
#include "core/util/spinlock.h"
#include "core/util/thread_info.h"

namespace bdm {
namespace memory_manager_detail {

struct Node {
  Node* next = nullptr;
};

/// List to store free memory regions. \n
/// Supports fast migration of N nodes to and from the list. \n
/// N has to be set when the object is constructed. \n
/// Fast migration is supported by maintaining a skip list.\n
class List {
 public:
  /// \param n n is the number of elements that can be added and removed very
  /// fast.
  explicit List(uint64_t n);

  List(const List& other);

  Node* PopFront();

  void PushFront(Node* head);

  void PushFrontThreadSafe(Node* head);

  void PushBackN(Node* head, Node* tail);

  void PushBackNThreadSafe(Node* head, Node* tail);

  void PopBackN(Node** head, Node** tail);

  void PopBackNThreadSafe(Node** head, Node** tail);

  bool Empty() const;

  bool CanPopBackN() const;

  uint64_t Size() const;

  uint64_t GetN() const;

 private:
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
  std::list<Node*> skip_list_;
  uint64_t size_ = 0;
  uint64_t nodes_before_skip_list_ = 0;
  /// Number of nodes for which fast migrations are supported
  uint64_t n_;
  Spinlock lock_;
};

/// Contains metadata for an allocated memory block.
struct AllocatedBlock {
  char* start_pointer_;
  char* end_pointer_;
  /// Memory to the left has been initialized.
  char* initialized_until_;

  bool IsFullyInitialized() const;

  void GetNextPageBatch(uint64_t size_n_pages, char** start, uint64_t* size);
};

/// Pool allocator for a specific allocation size and numa node. \n
class NumaPoolAllocator {
 public:
  static uint64_t RoundUpTo(uint64_t number, uint64_t multiple);

  NumaPoolAllocator(uint64_t size, int nid, uint64_t size_n_pages,
                    double growth_rate, uint64_t max_mem_per_thread);

  ~NumaPoolAllocator();

  void* New(int tid);

  void Delete(void* p);

  uint64_t GetSize() const;

 private:
  static constexpr uint64_t kMetadataSize = 8;
  uint64_t size_n_pages_;
  double growth_rate_;
  uint64_t max_nodes_per_thread_;
  uint64_t num_elements_per_n_pages_;
  uint64_t total_size_ = 0;
  uint64_t size_;
  int nid_;
  ThreadInfo* tinfo_;
  std::vector<AllocatedBlock> memory_blocks_;
  std::vector<List> free_lists_;  // one per thread
  List central_;
  Spinlock lock_;

  void AllocNewMemoryBlock(std::size_t size);

  void InitializeNPages(List* tl_list, char* block, uint64_t mem_block_size);
};

class PoolAllocator {
 public:
  PoolAllocator(std::size_t size, uint64_t size_n_pages, double growth_rate,
                uint64_t max_mem_per_thread);

  PoolAllocator(PoolAllocator&& other);
  PoolAllocator(const PoolAllocator& other) = delete;

  ~PoolAllocator();

  void* New(std::size_t size);

 private:
  std::size_t size_;
  ThreadInfo* tinfo_;
  std::vector<NumaPoolAllocator*> numa_allocators_;
};

}  // namespace memory_manager_detail

class MemoryManager {
 public:
  MemoryManager(uint64_t aligned_pages_shift, double growth_rate,
                uint64_t max_mem_per_thread);

  ~MemoryManager();

  void* New(std::size_t size);

  void Delete(void* p);

 private:
  double growth_rate_;
  uint64_t max_mem_per_thread_;
  uint64_t page_size_;
  uint64_t page_shift_;
  uint64_t aligned_pages_shift_;
  uint64_t aligned_pages_;
  uint64_t size_n_pages_;
  uint64_t num_threads_;

  UnorderedFlatmap<std::size_t, memory_manager_detail::PoolAllocator*>
      allocators_;

  Spinlock lock_;
};

}  // namespace bdm

#endif  // CORE_MEMORY_MEMORY_MANAGER_H_
