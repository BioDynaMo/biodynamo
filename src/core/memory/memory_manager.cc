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
  assert(false);
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
NumaPoolAllocator::NumaPoolAllocator(uint64_t size, int nid)
    : size_(size), nid_(nid), tinfo_(ThreadInfo::GetInstance()) {
  free_lists_.resize(tinfo_->GetThreadsInNumaNode(nid));
  AllocNewMemoryBlock(40960);
}

NumaPoolAllocator::~NumaPoolAllocator() {
  for (auto& block : memory_blocks_) {
    numa_free(block.first, block.second);
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
    AllocNewMemoryBlock(total_size_ * (kGrowthFactor - 1.0) );  // FIXME growth factor - grows a lot faster
    return New(ntid);
  }
}

void NumaPoolAllocator::Delete(void* p) {
  auto* node = new (p) Node();
  if (tinfo_->GetMyNumaNode() == nid_) {
    central_.Push(node, nullptr);
  } else {
    auto tid = tinfo_->GetMyThreadId();
    free_lists_[tid].Push(node, nullptr);
  }
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
  ProcessPages(block, size);
  assert(central_.Empty()); // FIXME remove
  central_.PushThreadSafe(head, tail);
}

void NumaPoolAllocator::CreateFreeList(void* block, uint64_t mem_block_size,
                                       Node** head, Node** tail) {
  auto* start_pointer = static_cast<char*>(block);
  auto* pointer = start_pointer;
  const auto* end_pointer = pointer + mem_block_size - size_;
  const uint64_t num_elements = mem_block_size / size_;
  // FIXME remove
  // std::cout << "block " << block << std::endl;
  // std::cout << "mbs   " << mem_block_size << std::endl;
  // std::cout << "size  " << size_ << std::endl;
  // std::cout << "ep    " << (void*)end_pointer << std::endl;
  // std::cout << "epo   " << end_pointer - static_cast<char*>(block) << std::endl;

  pointer += (num_elements - 1) * size_;
  *head = new (pointer) Node();
  assert((*head)->next == nullptr);
  *tail = *head;
  pointer -= size_;

  while (pointer >= start_pointer) {
    auto* old_head = *head;
    *head = new (pointer) Node();
    assert(pointer >= static_cast<char*>(block));
    assert(pointer <= end_pointer);
    assert((*head)->next == nullptr);
    (*head)->next = old_head;
    pointer -= size_;
  }

  // FIXME remove
  // assert((*tail)->next == nullptr);
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
  // assert(n == *tail);
  // FIXME remove end
}

void NumaPoolAllocator::ProcessPages(void* block, uint64_t mem_block_size) {
  auto num_pages = mem_block_size >> MemoryManager::kPageShift;

  auto i = reinterpret_cast<uint64_t>(block);
  auto start_page_number = i >> MemoryManager::kPageShift; // = i / kPageSize
  for (uint64_t i = 0; i < num_pages; i++) {
    MemoryManager::page_tree_.AddPage(start_page_number + i, this);
  }
}

// -----------------------------------------------------------------------------
TreeNode::TreeNode(uint64_t num_elements) {
  nodes_.resize(num_elements);
}

// -----------------------------------------------------------------------------
PageTree::PageTree(uint64_t total_memory, uint64_t page_shift) {
  max_number_pages_ = std::ceil(total_memory >> page_shift);
  num_elements_per_node_ = static_cast<uint64_t>(std::ceil(std::cbrt(max_number_pages_)));

  auto first_non_zero_bit = static_cast<uint64_t>(std::ceil(std::log2(max_number_pages_)));
  bits_per_index_ = first_non_zero_bit / 3;
  bits_per_index_ += first_non_zero_bit % 3 == 0 ? 0 : 1;

  idx0_mask_ = (1 << bits_per_index_) - 1;
  idx1_mask_ = idx0_mask_ << bits_per_index_;
  idx2_mask_ = idx1_mask_ << bits_per_index_;

  head_ = new TreeNode(num_elements_per_node_);
}

NumaPoolAllocator* PageTree::GetAllocator(uint64_t page_number) const {
  const auto& indices =  GetIndices(page_number);
  auto* node_level1 =  head_->nodes_[indices[0]].node_;
  auto* node_level2 =  node_level1->nodes_[indices[1]].node_;
  return node_level2->nodes_[indices[2]].data_;
}

void PageTree::AddPage(uint64_t page_number, NumaPoolAllocator* npa) {
  const auto& indices =  GetIndices(page_number);
  auto* node_level1 =  head_->nodes_[indices[0]].node_;
  if (node_level1 == nullptr) {
    std::lock_guard<Spinlock> guard_(head_->lock_);
    // check again if another thread has added a TreeNode in the meantime
    if (head_->nodes_[indices[0]].node_ == nullptr) {
      node_level1 = new TreeNode(num_elements_per_node_);
      head_->nodes_[indices[0]].node_ = node_level1;
    } else {
      node_level1 = head_->nodes_[indices[0]].node_;
    }
  }
  auto* node_level2 =  node_level1->nodes_[indices[1]].node_;
  if (node_level2 == nullptr) {
    std::lock_guard<Spinlock> guard_(node_level1->lock_);
    // check again if another thread has added a TreeNode in the meantime
    if (node_level1->nodes_[indices[1]].node_ == nullptr) {
      node_level2 = new TreeNode(num_elements_per_node_);
      node_level1->nodes_[indices[1]].node_ = node_level2;
    } else {
      node_level2 = node_level1->nodes_[indices[1]].node_;
    }
  }
  node_level2->nodes_[indices[2]].data_ = npa;
}

std::array<uint64_t, 3> PageTree::GetIndices(uint64_t page_number) const {
  std::array<uint64_t, 3> indices;

  indices[0] = page_number & idx0_mask_;
  indices[1] = page_number & idx1_mask_ >> bits_per_index_;
  indices[2] = page_number & idx2_mask_ >> (bits_per_index_ + 1);

  assert(indices[0] < num_elements_per_node_);
  assert(indices[1] < num_elements_per_node_);
  assert(indices[2] < num_elements_per_node_);
  return indices;
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
  auto i = reinterpret_cast<uint64_t>(p);
  auto page_number = i >> MemoryManager::kPageShift; // = i / kPageSize
  auto npa = MemoryManager::page_tree_.GetAllocator(page_number);
  npa->Delete(p);
}

std::unordered_map<std::size_t, PoolAllocator> MemoryManager::allocators_;
// TODO read out total memory
PageTree MemoryManager::page_tree_(static_cast<uint64_t>(std::pow(1024, 4)), MemoryManager::kPageShift);

}  // namespace bdm
