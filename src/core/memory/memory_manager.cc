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
  }
}

void List::Push(Node* head, Node* tail) {
  std::lock_guard<Spinlock> guard(lock_);
  if (head == nullptr) {
    return;
  }
  auto* old_head = head_;
  head_ = head;
  if (old_head == nullptr) {
    return;
  }
  if (tail == nullptr) {
    head->next = old_head->next;
  } else {
    tail->next = old_head->next;
  }
}

void List::PushThreadSafe(Node* head, Node* tail) {
  std::lock_guard<Spinlock> guard(lock_);
  Push(head, tail);
}

bool List::Empty() const { return head_ == nullptr; }

NumaPoolAllocator::NumaPoolAllocator(uint64_t size, int nid)
    : size_(size), nid_(nid) {
  free_lists_.resize(ThreadInfo::GetInstance()->GetThreadsInNumaNode(nid));
  AllocNewMemoryBlock(40960);
}

NumaPoolAllocator::~NumaPoolAllocator() {
  for (auto& block : memory_blocks_) {
    numa_free(block.first, block.second);
  }
}

void* NumaPoolAllocator::New(int tid) {
  auto& tl_list = free_lists_[tid];
  if (!tl_list.Empty()) {
    return tl_list.Pop();
  } else if (!central_.Empty()) {
    Node *head = nullptr, *tail = nullptr;
    central_.PopNThreadSafe(1000, &head, &tail);  // FIXME hardcoded value
    tl_list.Push(head, tail);
    return tl_list.Pop();
  } else {
    AllocNewMemoryBlock(total_size_ * kGrowthFactor);
    return New(tid);
  }
}

void NumaPoolAllocator::Delete(void* p) {
  // TODO
}

void NumaPoolAllocator::AllocNewMemoryBlock(std::size_t size) {
  std::lock_guard<std::mutex> guard(mutex_);
  // check again if it is empty or if an allocation happened while a thread
  // was put to sleep by the lock above.
  // FIXME problematic if an object gets freed before first thread gets to
  // here
  if (!central_.Empty()) {
    return;
  }

  void* block = numa_alloc_onnode(size, nid_);
  if (block == nullptr) {
    Log::Fatal("NumaPoolAllocator::AllocNewMemoryBlock", "Allocation failed");
  }
  total_size_ += size;
  memory_blocks_.push_back({block, size});
  Node *head = nullptr, *tail = nullptr;
  CreateFreeList(block, size, &head, &tail);
  central_.PushThreadSafe(head, tail);
}

void NumaPoolAllocator::CreateFreeList(void* block, uint64_t mem_block_size,
                                       Node** head, Node** tail) {
  auto* pointer = static_cast<char*>(block);
  *head = new (pointer) Node();
  *tail = *head;
  pointer += size_;
  auto* end_pointer = pointer + mem_block_size;
  while (pointer < end_pointer) {
    auto* old_head = *head;
    *head = new (pointer) Node();
    (*head)->next = old_head;
    pointer += size_;
  }
}

PoolAllocator::PoolAllocator(std::size_t size)
    : size_(size), tinfo_(ThreadInfo::GetInstance()) {
  for (int nid = 0; nid < tinfo_->GetNumaNodes(); ++nid) {
    numa_allocators_.push_back(new NumaPoolAllocator(size, nid));
  }
}

PoolAllocator::~PoolAllocator() {}

void* PoolAllocator::New(std::size_t size) {
  assert(size_ == size && "Requested size does not match this PoolAllocator");
  auto tid = tinfo_->GetMyThreadId();
  auto nid = tinfo_->GetNumaNode(tid);
  assert(nid < numa_allocators_.size());
  return numa_allocators_[nid]->New(tid);
}

void PoolAllocator::Delete(void* p) {
  // TODO
}

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
  // TODO
}

}  // namespace bdm
