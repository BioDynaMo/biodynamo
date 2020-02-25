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
#include <cmath>
#include <mutex>

namespace bdm {

Node* List::Pop() {
  if (!Empty()) {
    auto* ret = head_;
    head_ = head_->next;
    return ret;
  }
  return nullptr;
}

void List::PopNThreadSafe(uint64_t n, Node** head, Node** tail) {
  std::lock_guard<Spinlock> guard(lock_);
  if (!Empty()) {
    *head = head_;
    *tail = (*head)->next;
    if (*tail == nullptr) {
      head_ = nullptr;
      return;
    }
    --n;
    while (--n > 0 && (*tail)->next != nullptr) {
      *tail = (*tail)->next;
    }
    head_ = (*tail)->next;
    (*tail)->next = nullptr;
  }
}

void List::Push(Node* head, Node* tail) {
  if (head == nullptr) {
    return;
  }
  auto* old_head = head_;
  head_ = head;
  if (old_head == nullptr) {
    return;
  }
  if (tail == nullptr) {
    head->next = old_head;
  } else {
    tail->next = old_head;
  }
}

void List::PushThreadSafe(Node* head, Node* tail) {
  std::lock_guard<Spinlock> guard(lock_);
  Push(head, tail);
}

bool List::Empty() const { return head_ == nullptr; }

// -----------------------------------------------------------------------------
bool AllocatedBlock::IsFullyInitialized() const {
  return initialized_until_ >= end_pointer_;
}

void AllocatedBlock::GetNextPageBatch(char** start, uint64_t* size) {
  *start = initialized_until_;
  initialized_until_ += MemoryManager::kSizeNPages;
  *size = MemoryManager::kSizeNPages;
  if(initialized_until_ > end_pointer_) {
    *size = end_pointer_ - *start;
  }
}

// -----------------------------------------------------------------------------
NumaPoolAllocator::NumaPoolAllocator(uint64_t size, int nid)
    : size_(size), nid_(nid), tinfo_(ThreadInfo::GetInstance()) {
  free_lists_.resize(tinfo_->GetThreadsInNumaNode(nid));
  AllocNewMemoryBlock(MemoryManager::kSizeNPages * 2);
}

NumaPoolAllocator::~NumaPoolAllocator() {
  for (auto& block : memory_blocks_) {
    uint64_t size = block.end_pointer_ - block.start_pointer_;
    numa_free(block.start_pointer_, size);
  }
}

void* NumaPoolAllocator::New(int ntid) {
  assert(ntid < free_lists_.size());
  auto& tl_list = free_lists_[ntid];
  if (!tl_list.Empty()) {
    return tl_list.Pop();
  } else if (!central_.Empty()) {
    Node *head = nullptr, *tail = nullptr;
    central_.PopNThreadSafe(1000, &head, &tail);  // FIXME hardcoded value
    if (head == nullptr) {
      return New(ntid);
    }
    tl_list.Push(head, tail);
    return tl_list.Pop();
  } else {
    lock_.lock();
    if (memory_blocks_.back().IsFullyInitialized()) {
      // FIXME growth factor - grows a lot faster
      auto size = total_size_ * (kGrowthFactor - 1.0);
      size = RoundUpTo(size, MemoryManager::kSizeNPages);
      AllocNewMemoryBlock(size);
    }
    char* start_pointer;
    uint64_t size;
    memory_blocks_.back().GetNextPageBatch(&start_pointer, &size);
    lock_.unlock();
    InitializeNPages(&tl_list, start_pointer, size);
    return tl_list.Pop();
  }
}

void NumaPoolAllocator::Delete(void* p) {
  auto* node = new (p) Node();
  if (tinfo_->GetMyNumaNode() == nid_) {
    auto ntid = tinfo_->GetMyNumaThreadId();
    free_lists_[ntid].Push(node, nullptr);
  } else {
    central_.PushThreadSafe(node, nullptr);
  }
}

void NumaPoolAllocator::AllocNewMemoryBlock(std::size_t size) {
  // check again if it is empty or if an allocation happened while a thread
  // was put to sleep by the lock above.
  // FIXME problematic if an object gets freed before first thread gets to
  // here
  // if (!central_.Empty()) {
  //   return;
  // }

  // TODO improve text
  // check if size is multiple of N pages aligned
  uint64_t size_n_pages = MemoryManager::kSizeNPages;
  assert((size & (size_n_pages -1)) == 0
    && "Size must be a multiple of page_size * num_pages_aligned");
  void* block = numa_alloc_onnode(size, nid_);
  if (block == nullptr) {
    Log::Fatal("NumaPoolAllocator::AllocNewMemoryBlock", "Allocation failed");
  }
  total_size_ += size;
  auto n_pages_aligned = RoundUpTo(reinterpret_cast<uint64_t>(block), size_n_pages);
  auto* start = reinterpret_cast<char*>(block);
  char* end = start + size;
  memory_blocks_.push_back({start, end, reinterpret_cast<char*>(n_pages_aligned)});

  // Node *head = nullptr, *tail = nullptr;
  // CreateFreeList(block, size, &head, &tail);
  // ProcessPages(block, size);
  // assert(central_.Empty()); // FIXME remove
  // central_.PushThreadSafe(head, tail);
}

void NumaPoolAllocator::InitializeNPages(List* tl_list, char* block, uint64_t mem_block_size) {
  // TODO assert N page aligned
  auto* block_npa = reinterpret_cast<NumaPoolAllocator**>(block);
  *block_npa = this;

  Node* head = nullptr;
  Node* tail = nullptr;

  // TODO cache line allign? 64
  uint64_t metadata_size = sizeof(NumaPoolAllocator*);
  auto* start_pointer = static_cast<char*>(block + metadata_size);
  auto* pointer = start_pointer;
  const uint64_t num_elements = (mem_block_size - metadata_size) / size_;
  // FIXME remove
  // const auto* end_pointer = pointer + mem_block_size - size_;
  // std::cout << "block " << block << std::endl;
  // std::cout << "mbs   " << mem_block_size << std::endl;
  // std::cout << "size  " << size_ << std::endl;
  // std::cout << "ep    " << (void*)end_pointer << std::endl;
  // std::cout << "epo   " << end_pointer - static_cast<char*>(block) << std::endl;

  pointer += (num_elements - 1) * size_;
  head = new (pointer) Node();
  assert(head->next == nullptr);
  tail = head;
  pointer -= size_;

  while (pointer >= start_pointer) {
    auto* old_head = head;
    head = new (pointer) Node();
    assert(pointer >= static_cast<char*>(block));
    // assert(pointer <= end_pointer);
    assert(head->next == nullptr);
    head->next = old_head;
    pointer -= size_;
  }

  // FIXME remove
  // assert(tail->next == nullptr);
  // Node* n = *head;
  // // std::cout << "    " << ((char*) n) - start_pointer << std::endl;
  // uint64_t cnt = 1;
  // while (n->next != nullptr) {
  //   n = n->next;
  //   // std::cout << "    " << ((char*) n) - start_pointer << " " << (void*) n << std::endl;
  //   assert((void*) n >= block);
  //   assert((char*) n <= end_pointer);
  //   cnt++;
  // }
  // assert(cnt == mem_block_size / size_);
  // assert(n == tail);
  // FIXME remove end

  tl_list->Push(head, tail);
}

uint64_t NumaPoolAllocator::RoundUpTo(uint64_t number, uint64_t multiple) {
  assert((multiple & (multiple - 1)) == 0 && multiple && "multiple must be a power of two and non-zero");
  return (number + multiple - 1) & -multiple;
}

// -----------------------------------------------------------------------------
PoolAllocator::PoolAllocator(std::size_t size)
    : size_(size), tinfo_(ThreadInfo::GetInstance()) {
  for (int nid = 0; nid < tinfo_->GetNumaNodes(); ++nid) {
    numa_allocators_.push_back(new NumaPoolAllocator(size, nid));
  }
}

PoolAllocator::PoolAllocator(const PoolAllocator& other)
    : size_(other.size_), tinfo_(ThreadInfo::GetInstance()) {
  // FIXME not a real copy ctor...
  // Alternative is unique_ptr
  for (int nid = 0; nid < tinfo_->GetNumaNodes(); ++nid) {
    numa_allocators_.push_back(new NumaPoolAllocator(size_, nid));
  }
}

PoolAllocator::~PoolAllocator() {
  for (auto* el : numa_allocators_) {
    delete el;
  }
  numa_allocators_.clear();
}

void* PoolAllocator::New(std::size_t size) {
  assert(size_ == size && "Requested size does not match this PoolAllocator");
  auto tid = tinfo_->GetMyThreadId();
  auto nid = tinfo_->GetNumaNode(tid);
  assert(static_cast<uint64_t>(nid) < numa_allocators_.size());
  return numa_allocators_[nid]->New(tinfo_->GetNumaThreadId(tid));
}

void PoolAllocator::Delete(void* p) {
  // TODO
}

// -----------------------------------------------------------------------------
void* MemoryManager::New(std::size_t size) {
  auto it = allocators_.find(size);
  if (it != allocators_.end()) {
    return it->second.New(size);
  } else {
    allocators_.emplace(size, PoolAllocator(size));
    return New(size);
  }
}

void MemoryManager::Delete(void* p) {
  auto addr = reinterpret_cast<uint64_t>(p);
  auto page_number = addr >> (MemoryManager::kPageShift + MemoryManager::kNumPagesAlignedShift);
  auto* page_addr = reinterpret_cast<char*>(page_number << (MemoryManager::kPageShift + MemoryManager::kNumPagesAlignedShift));
  auto* npa = *reinterpret_cast<NumaPoolAllocator**>(page_addr);
  npa->Delete(p);
}

std::unordered_map<std::size_t, PoolAllocator> MemoryManager::allocators_;
constexpr uint64_t MemoryManager::kSizeNPages;
}  // namespace bdm
