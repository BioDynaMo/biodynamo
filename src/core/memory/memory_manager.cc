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
#include <cstdlib>
#include <mutex>

namespace bdm {

List::List(uint64_t n) : n_(n) {
  // TODO check if n_ is power of two -> otherwise fatal
}

// TODO
List::List(const List& other) : head_(other.head_)
{}

Node* List::PopFront() {
  if (!Empty()) {
    auto* ret = head_;
    head_ = head_->next;
    if (tail_ == ret) {
      tail_ = nullptr;
    }
    --size_;
    --nodes_before_skip_list_;
    if (skip_list_.back() == ret) {
      skip_list_.pop_back();
      nodes_before_skip_list_ = n_;
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

  // (size_ + 1) mod n == 0
  // if (((size_ - 1) & (n_ - 1)) == 0 && size_ > n_) {
  if (nodes_before_skip_list_ >= n_ && size_ > n_) {
    skip_list_.push_back(head_);
    nodes_before_skip_list_ = 0;
  }
}

void List::PushFrontThreadSafe(Node* head) {
  std::lock_guard<Spinlock> guard(lock_);
  // #pragma omp critical
  // std::cout << this << " push front" << std::endl;
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
  size_+= n_;
}

void List::PushBackNThreadSafe(Node* head, Node* tail) {
  std::lock_guard<Spinlock> guard(lock_);
  // #pragma omp critical
  // std::cout << this << " push back N" << std::endl;
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

  // if (!Empty()) {
  //   *head = head_;
  //   *tail = (*head)->next;
  //   if (*tail == nullptr) {
  //     head_ = nullptr;
  //     return;
  //   }
  //   --n;
  //   while (--n > 0 && (*tail)->next != nullptr) {
  //     *tail = (*tail)->next;
  //   }
  //   head_ = (*tail)->next;
  //   (*tail)->next = nullptr;
  // }
}

void List::PopBackNThreadSafe(Node** head, Node** tail) {
  std::lock_guard<Spinlock> guard(lock_);
  // #pragma omp critical
  // std::cout << this << " pop back N "  << " size " << size_  << " sk-size " << skip_list_.size() << " nbsk " << nodes_before_skip_list_ << std::endl;
  PopBackN(head, tail);
}

bool List::Empty() const { return head_ == nullptr; }

bool List::CanPopBackN() const { return skip_list_.size() != 0; }

uint64_t List::Size() const { return size_; }

uint64_t List::GetN() const { return n_; }

void List::SetN(uint64_t n)  { n_ = n; }

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
  // TODO remove code duplication - 8 is the size of the metadata at the
  // beginning of each N aligned pages - duplicated in InializeNPages
  auto num_elements_fast_migrate = (MemoryManager::kSizeNPages - 8) / size;
  free_lists_.reserve(tinfo_->GetThreadsInNumaNode(nid));
  for (int i = 0; i < tinfo_->GetThreadsInNumaNode(nid); ++i) {
    free_lists_.emplace_back(num_elements_fast_migrate);
  }
  central_.SetN(num_elements_fast_migrate);
  AllocNewMemoryBlock(MemoryManager::kSizeNPages * 2);
}

NumaPoolAllocator::~NumaPoolAllocator() {
  for (auto& block : memory_blocks_) {
    uint64_t size = block.end_pointer_ - block.start_pointer_;
#ifdef __APPLE__
    free(block.start_pointer_);
#else
    numa_free(block.start_pointer_, size);
#endif  // __APPLE__
  }
}

void* NumaPoolAllocator::New(int ntid) {
  assert(static_cast<uint64_t>(ntid) < free_lists_.size());
  auto& tl_list = free_lists_[ntid];
  if (!tl_list.Empty()) {
    auto *ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
  } else if (central_.CanPopBackN()) {
    Node *head = nullptr, *tail = nullptr;
    central_.PopBackNThreadSafe(&head, &tail);
    if (head == nullptr) {
      return New(ntid);
    }
    tl_list.PushBackN(head, tail);
    auto *ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
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
    auto *ret = tl_list.PopFront();
    assert(ret != nullptr);
    return ret;
  }
}

void NumaPoolAllocator::Delete(void* p) {
  auto* node = new (p) Node();
  if (tinfo_->GetMyNumaNode() == nid_) {
    auto ntid = tinfo_->GetMyNumaThreadId();
    free_lists_[ntid].PushFront(node);
  } else {
    central_.PushFrontThreadSafe(node);
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
#ifdef __APPLE__
  void* block = malloc(size);
#else
  void* block = numa_alloc_onnode(size, nid_);
#endif  // __APPLE__
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

  // TODO cache line allign? 64
  uint64_t metadata_size = sizeof(NumaPoolAllocator*);
  auto* start_pointer = static_cast<char*>(block + metadata_size);
  auto* pointer = start_pointer;
  const uint64_t num_elements = (mem_block_size - metadata_size) / size_;
  // FIXME remove
  // const auto* end_pointer = start_pointer + mem_block_size - size_;
  // std::cout << "block " << block << std::endl;
  // std::cout << "mbs   " << mem_block_size << std::endl;
  // std::cout << "size  " << size_ << std::endl;
  // std::cout << "ep    " << (void*)end_pointer << std::endl;
  // std::cout << "epo   " << end_pointer - static_cast<char*>(block) << std::endl;

  pointer += (num_elements - 1) * size_;
  if (tl_list->GetN() == num_elements) {

    auto* head = new (pointer) Node();
    assert(head->next == nullptr);
    auto* tail = head;
    pointer -= size_;

    while (pointer >= start_pointer) {
      auto* old_head = head;
      head = new (pointer) Node();
      assert(pointer >= static_cast<char*>(block));
      assert(pointer <= (start_pointer + mem_block_size - size_));
      assert(head->next == nullptr);
      head->next = old_head;
      pointer -= size_;
    }
    tl_list->PushBackN(head, tail);

  } else {
    while (pointer >= start_pointer) {
      assert(pointer >= static_cast<char*>(block));
      assert(pointer <= start_pointer + mem_block_size - size_);
      tl_list->PushFront(new (pointer) Node());
      pointer -= size_;
    }
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
