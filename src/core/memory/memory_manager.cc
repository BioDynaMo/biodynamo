// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <mutex>
#include "core/util/log.h"
#include "core/util/string.h"

namespace bdm {
namespace memory_manager_detail {

List::List(uint64_t n) : n_(n) {}

List::List(const List& other)
    : head_(other.head_),
      tail_(other.tail_),
      skip_list_(other.skip_list_),
      size_(other.size_),
      nodes_before_skip_list_(other.nodes_before_skip_list_),
      n_(other.n_) {}

Node* List::PopFront() {
  if (!Empty()) {
    auto* ret = head_;
    head_ = head_->next;
    if (head_ != nullptr) {
      __builtin_prefetch(head_->next);
    }
    if (tail_ == ret) {
      tail_ = nullptr;
    }
    --size_;
    if (skip_list_.back() == ret) {
      skip_list_.pop_back();
      nodes_before_skip_list_ = n_;
    } else if (size_ != 0) {
      --nodes_before_skip_list_;
    }
    return ret;
  }
  return nullptr;
}

void List::PushFront(Node* head) {
  assert(head != nullptr);

  auto* old_head = head_;
  head_ = head;
  head_->next = old_head;
  if (old_head == nullptr) {
    tail_ = head_;
  }
  ++size_;
  ++nodes_before_skip_list_;

  if (nodes_before_skip_list_ >= n_ && size_ > n_) {
    skip_list_.push_back(head_);
    nodes_before_skip_list_ = 0;
  }
}

void List::PushFrontThreadSafe(Node* head) {
  std::lock_guard<Spinlock> guard(lock_);
  PushFront(head);
}

void List::PushBackN(Node* head, Node* tail) {
  assert(head != nullptr);
  assert(tail != nullptr);

  if (head_ == nullptr) {
    head_ = head;
    tail_ = tail;
    size_ = n_;
    nodes_before_skip_list_ = n_;
    return;
  }

  skip_list_.push_front(tail_);
  tail_->next = head;
  tail_ = tail;
  size_ += n_;
}

void List::PushBackNThreadSafe(Node* head, Node* tail) {
  std::lock_guard<Spinlock> guard(lock_);
  PushBackN(head, tail);
}

void List::PopBackN(Node** head, Node** tail) {
  assert(head_ != nullptr);
  assert(tail_ != nullptr);

  if (skip_list_.size() == 0) {
    return;
  }

  *head = skip_list_.front()->next;
  skip_list_.front()->next = nullptr;
  *tail = tail_;
  tail_ = skip_list_.front();
  skip_list_.pop_front();
  size_ -= n_;
}

void List::PopBackNThreadSafe(Node** head, Node** tail) {
  std::lock_guard<Spinlock> guard(lock_);
  PopBackN(head, tail);
}

bool List::Empty() const { return head_ == nullptr; }

bool List::CanPopBackN() const { return skip_list_.size() != 0; }

uint64_t List::Size() const { return size_; }

uint64_t List::GetN() const { return n_; }

// -----------------------------------------------------------------------------
bool AllocatedBlock::IsFullyInitialized() const {
  return initialized_until_ >= end_pointer_;
}

void AllocatedBlock::GetNextPageBatch(uint64_t size_n_pages, char** start,
                                      uint64_t* size) {
  *start = initialized_until_;
  initialized_until_ += size_n_pages;
  *size = size_n_pages;
  if (initialized_until_ > end_pointer_) {
    *size = end_pointer_ - *start;
  }
}

// -----------------------------------------------------------------------------
NumaPoolAllocator::NumaPoolAllocator(uint64_t size, int nid,
                                     uint64_t size_n_pages, double growth_rate,
                                     uint64_t max_mem_per_thread)
    : size_n_pages_(size_n_pages),
      growth_rate_(growth_rate),
      max_nodes_per_thread_(max_mem_per_thread / size),
      num_elements_per_n_pages_((size_n_pages_ - kMetadataSize) / size),
      size_(size),
      nid_(nid),
      tinfo_(ThreadInfo::GetInstance()),
      central_(num_elements_per_n_pages_) {
  free_lists_.reserve(tinfo_->GetMaxThreads());
  for (int i = 0; i < tinfo_->GetMaxThreads(); ++i) {
    free_lists_.emplace_back(num_elements_per_n_pages_);
  }
}

NumaPoolAllocator::~NumaPoolAllocator() {
  for (auto& block : memory_blocks_) {
    uint64_t size = block.end_pointer_ - block.start_pointer_;
    numa_free(block.start_pointer_, size);
  }
}

void* NumaPoolAllocator::New(int tid) {
  assert(static_cast<uint64_t>(tid) < free_lists_.size());
  auto& tl_list = free_lists_[tid];
  if (!tl_list.Empty()) {
    auto* ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
  } else if (central_.CanPopBackN()) {
    Node *head = nullptr, *tail = nullptr;
    central_.PopBackNThreadSafe(&head, &tail);
    if (head == nullptr) {
      return New(tid);
    }
    tl_list.PushBackN(head, tail);
    auto* ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
  } else {
    lock_.lock();
    if (memory_blocks_.size() == 0 ||
        memory_blocks_.back().IsFullyInitialized()) {
      auto size =
          std::max(total_size_ * (growth_rate_ - 1.0), size_n_pages_ * 2.0);
      size = RoundUpTo(size, size_n_pages_);
      AllocNewMemoryBlock(size);
    }
    char* start_pointer;
    uint64_t size;
    memory_blocks_.back().GetNextPageBatch(size_n_pages_, &start_pointer,
                                           &size);
    lock_.unlock();
    // remaining memory not enough to store one element
    if ((size - kMetadataSize) < size_) {
      return New(tid);
    }
    InitializeNPages(&tl_list, start_pointer, size);
    auto* ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
  }
}

void NumaPoolAllocator::Delete(void* p) {
  auto* node = new (p) Node();
  auto tid = tinfo_->GetMyThreadId();
  auto& tl_list = free_lists_[tid];
  tl_list.PushFront(node);
  // migrate too much unused memory to the central list agent other threads
  // can obtain it
  if (tl_list.Size() > max_nodes_per_thread_ && tl_list.CanPopBackN()) {
    Node* head = nullptr;
    Node* tail = nullptr;
    tl_list.PopBackN(&head, &tail);
    central_.PushBackNThreadSafe(head, tail);
  }
}

uint64_t NumaPoolAllocator::GetSize() const { return size_; }

void NumaPoolAllocator::AllocNewMemoryBlock(std::size_t size) {
  // check if size is multiple of N pages aligned
  assert((size & (size_n_pages_ - 1)) == 0 &&
         "Size must be a multiple of MemoryManager::kSizeNPages");
  void* block = numa_alloc_onnode(size, nid_);
  if (block == nullptr) {
    Log::Fatal("NumaPoolAllocator::AllocNewMemoryBlock", "Allocation failed");
  }
  total_size_ += size;
  auto n_pages_aligned =
      RoundUpTo(reinterpret_cast<uint64_t>(block), size_n_pages_);
  auto* start = reinterpret_cast<char*>(block);
  char* end = start + size;
  memory_blocks_.push_back(
      {start, end, reinterpret_cast<char*>(n_pages_aligned)});
}

void NumaPoolAllocator::InitializeNPages(List* tl_list, char* block,
                                         uint64_t mem_block_size) {
  assert((reinterpret_cast<uint64_t>(block) & (size_n_pages_ - 1)) == 0 &&
         "block is not N page aligned");
  auto* block_npa = reinterpret_cast<NumaPoolAllocator**>(block);
  *block_npa = this;

  auto* start_pointer = static_cast<char*>(block + kMetadataSize);
  auto* pointer = start_pointer;
  const uint64_t num_elements = (mem_block_size - kMetadataSize) / size_;

  if (tl_list->GetN() == num_elements) {
    auto* head = new (pointer) Node();
    assert(head->next == nullptr);
    auto* tail = head;

    for (uint64_t i = 1; i < num_elements; ++i) {
      assert(pointer >= static_cast<char*>(block));
      assert(pointer <= start_pointer + mem_block_size - size_);
      __builtin_prefetch(pointer + size_);
      auto* node = new (pointer) Node();
      tail->next = node;
      tail = node;
      pointer += size_;
    }
    tl_list->PushBackN(head, tail);
  } else {
    for (uint64_t i = 0; i < num_elements; ++i) {
      assert(pointer >= static_cast<char*>(block));
      assert(pointer <= start_pointer + mem_block_size - size_);
      __builtin_prefetch(pointer + size_);
      tl_list->PushFront(new (pointer) Node());
      pointer += size_;
    }
  }
}

uint64_t NumaPoolAllocator::RoundUpTo(uint64_t number, uint64_t multiple) {
  assert((multiple & (multiple - 1)) == 0 && multiple &&
         "multiple must be a power of two and non-zero");
  return (number + multiple - 1) &
         (std::numeric_limits<uint64_t>::max() - (multiple - 1));
}

// -----------------------------------------------------------------------------
PoolAllocator::PoolAllocator(std::size_t size, uint64_t size_n_pages,
                             double growth_rate, uint64_t max_mem_per_thread)
    : size_(size), tinfo_(ThreadInfo::GetInstance()) {
  for (int nid = 0; nid < tinfo_->GetNumaNodes(); ++nid) {
    void* ptr = numa_alloc_onnode(sizeof(NumaPoolAllocator), nid);
    numa_allocators_.push_back(new (ptr) NumaPoolAllocator(
        size, nid, size_n_pages, growth_rate, max_mem_per_thread));
  }
}

PoolAllocator::PoolAllocator(PoolAllocator&& other)
    : size_(other.size_),
      tinfo_(other.tinfo_),
      numa_allocators_(std::move(other.numa_allocators_)) {}

PoolAllocator::~PoolAllocator() {
  for (auto* el : numa_allocators_) {
    el->~NumaPoolAllocator();
    numa_free(el, sizeof(NumaPoolAllocator));
  }
  numa_allocators_.clear();
}

void* PoolAllocator::New(std::size_t size) {
  assert(size_ == size && "Requested size does not match this PoolAllocator");
  auto tid = tinfo_->GetMyThreadId();
  auto nid = tinfo_->GetNumaNode(tid);
  assert(static_cast<uint64_t>(nid) < numa_allocators_.size());
  return numa_allocators_[nid]->New(tid);
}

}  // namespace memory_manager_detail

// -----------------------------------------------------------------------------
MemoryManager::MemoryManager(uint64_t aligned_pages_shift, double growth_rate,
                             uint64_t max_mem_per_thread)
    : growth_rate_(growth_rate),
      max_mem_per_thread_(max_mem_per_thread),
      page_size_(sysconf(_SC_PAGESIZE)),
      page_shift_(static_cast<uint64_t>(std::log2(page_size_))),
      num_threads_(ThreadInfo::GetInstance()->GetMaxThreads()) {
  aligned_pages_shift_ = aligned_pages_shift;
  aligned_pages_ = (1 << aligned_pages_shift_);
  size_n_pages_ = (1 << (page_shift_ + aligned_pages_shift_));

  if (max_mem_per_thread_ <= size_n_pages_) {
    Log::Fatal(
        "MemoryManager",
        Concat("The parameter mem_mgr_max_mem_per_thread must be greater then "
               "the size of N pages (PAGE_SIZE * 2 ^ mem_mgr_growth_rate)! "
               "(max_mem_per_thread_ ",
               max_mem_per_thread_, ", size_n_pages_ ", size_n_pages_, ")"));
  }

  allocators_.reserve(num_threads_ * 2 + 100);
}

MemoryManager::~MemoryManager() {
  for (auto& pair : allocators_) {
    delete pair.second;
  }
}

void* MemoryManager::New(std::size_t size) {
  if (allocators_.Capacity() > num_threads_) {
    auto it = allocators_.find(size);
    if (it != allocators_.end()) {
      return it->second->New(size);
    } else {
      std::lock_guard<Spinlock> guard(lock_);
      // check again, another thread might have created it in between
      if (allocators_.find(size) == allocators_.end()) {
        allocators_.insert(std::make_pair(
            size, new memory_manager_detail::PoolAllocator(
                      size, size_n_pages_, growth_rate_, max_mem_per_thread_)));
      }
      return New(size);
    }
  } else {
    std::lock_guard<Spinlock> guard(lock_);
    auto it = allocators_.find(size);
    if (it != allocators_.end()) {
      return it->second->New(size);
    } else {
      allocators_.insert(std::make_pair(
          size, new memory_manager_detail::PoolAllocator(
                    size, size_n_pages_, growth_rate_, max_mem_per_thread_)));
      return allocators_.find(size)->second;
    }
  }
}

void MemoryManager::Delete(void* p) {
  if (ignore_delete_) {
    return;
  }
  auto addr = reinterpret_cast<uint64_t>(p);
  auto page_number = addr >> (page_shift_ + aligned_pages_shift_);
  auto* page_addr = reinterpret_cast<char*>(
      page_number << (page_shift_ + aligned_pages_shift_));
  auto* npa =
      *reinterpret_cast<memory_manager_detail::NumaPoolAllocator**>(page_addr);
  npa->Delete(p);
}

void MemoryManager::SetIgnoreDelete(bool value) { ignore_delete_ = value; }

}  // namespace bdm
